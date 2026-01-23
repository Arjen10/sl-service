//
// Created by Arjen on 2024/12/21.
//

#ifndef SL_BASIC_HPP
#define SL_BASIC_HPP

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core.hpp>
#include <boost/asio/coroutine.hpp>
#include <memory>
#include <thread>
#include <string>

#include "sl_session_map.hpp"

namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

class sl_listener : public std::enable_shared_from_this<sl_listener> {
    net::io_context& ioc_;
    tcp::acceptor acceptor_;

  public:
    sl_listener(net::io_context& ioc, const tcp::endpoint& endpoint);
    void run();

  private:
    void do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);
};

#endif // SL_BASIC_HPP
