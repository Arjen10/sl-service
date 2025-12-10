//
// Created by Arjen on 2024/12/21.
//

#include "sl_listener.hpp"

#include "../core/log.hpp"

sl_listener::sl_listener(net::io_context &ioc, const tcp::endpoint& endpoint)
        : ioc_(ioc), acceptor_(net::make_strand(ioc)) {
    beast::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        LOG_ERROR <<  ec.message() << " open";
        return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
        LOG_ERROR <<  ec.message() << " set_option";
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if (ec) {
        LOG_ERROR <<  ec.message() << " bind";
        return;
    }

    // Start listening for connections
    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        LOG_ERROR <<  ec.message() << " listen";
        return;
    }
}

void sl_listener::run() {
    net::dispatch(acceptor_.get_executor(),
                  beast::bind_front_handler(&sl_listener::do_accept, this->shared_from_this()));
}

void sl_listener::do_accept() {
    acceptor_.async_accept(net::make_strand(ioc_),
                           beast::bind_front_handler(&sl_listener::on_accept, this->shared_from_this()));
}

void sl_listener::on_accept(beast::error_code ec, tcp::socket socket) {
    LOG_DEBUG << socket.remote_endpoint().address().to_string() << " accept";
    if (ec) {
        LOG_ERROR << ec.message() << " accept";
    } else {
        // 创建一个水利会话，并运行
        auto session_ptr = std::make_shared<sl_session>(std::move(socket), this->ioc_);
        session_ptr->run();
        //sl_session_map::instance().thread_safe_insert(session_ptr);
    }

    // 接受其它的链接
    do_accept();
}
