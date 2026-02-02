//
// Created by Arjen on 2025/10/30.
//

#include "sl_session.hpp"

#include "../core/sys_thread_pool.hpp"

bool parser_is_done::operator()(const sl_req_decoder& p) const {
    return p.is_done();
}

template <class AsyncReadStream, class DynamicBuffer>
read_some_op<AsyncReadStream, DynamicBuffer>::read_some_op(AsyncReadStream& s, DynamicBuffer& b, sl_req_decoder& p)
    : s_(s), b_(b), p_(p), bytes_transferred_(0), cont_(false) {
}

template <class AsyncReadStream, class DynamicBuffer>
template <class Self>
void read_some_op<AsyncReadStream, DynamicBuffer>::operator()(Self& self, beast::error_code ec,
                                                              std::size_t bytes_transferred) {
    BOOST_ASIO_CORO_REENTER(*this) {
        if (b_.size() == 0)
            goto do_read;
        for (;;) {
            // parse
            {
                auto const used = p_.put(b_.data(), ec);
                bytes_transferred_ += used;
                b_.consume(used);
            }
            if (ec != http::error::need_more)
                break;

        do_read:
            BOOST_ASIO_CORO_YIELD {
                cont_ = true;
                const auto& session = conf::sl::instance().get_server().get_session();
                auto const size = boost::beast::read_size(b_, session.get_max_buffer());
                if (size == 0) {
                    BOOST_BEAST_ASSIGN_EC(ec, http::error::buffer_overflow);
                    goto upcall;
                }
                auto const mb = beast::detail::dynamic_buffer_prepare(b_, size, ec, http::error::buffer_overflow);
                if (ec)
                    goto upcall;

                BOOST_ASIO_HANDLER_LOCATION((__FILE__, __LINE__, "sl::async_read_some"));

                s_.async_read_some(*mb, std::move(self));
            }
            b_.commit(bytes_transferred);
            if (ec == net::error::eof) {
                BOOST_ASSERT(bytes_transferred == 0);
                break;
            }
            if (ec)
                break;
        }

    upcall:
        if (!cont_) {
            BOOST_ASIO_CORO_YIELD {
                BOOST_ASIO_HANDLER_LOCATION((__FILE__, __LINE__, "sl::async_read_some"));
                net::post(s_.get_executor(), beast::bind_front_handler(std::move(self), ec));
            }
        }
        self.complete(ec, bytes_transferred_);
    }
}

template <class AsyncReadStream, class DynamicBuffer, typename ReadHandler>
BOOST_BEAST_ASYNC_RESULT2(ReadHandler)
async_read_some(AsyncReadStream& stream, DynamicBuffer& buffer, sl_req_decoder& parser, ReadHandler&& handler) {
    return net::async_compose<ReadHandler, void(beast::error_code, std::size_t)>(
        read_some_op<AsyncReadStream, DynamicBuffer>{stream, buffer, parser}, handler, stream);
}

template <class Stream, class DynamicBuffer, class Condition>
read_op<Stream, DynamicBuffer, Condition>::read_op(Stream& s, DynamicBuffer& b, sl_req_decoder& p)
    : s_(s), b_(b), p_(p), bytes_transferred_(0) {
}

template <class Stream, class DynamicBuffer, class Condition>
template <class Self>
void read_op<Stream, DynamicBuffer, Condition>::operator()(Self& self, boost::beast::error_code ec,
                                                           std::size_t bytes_transferred) {
    BOOST_ASIO_CORO_REENTER(*this) {
        // 如果已经解析完成了，则交给asio完成回调
        if (Condition{}(p_)) {
            BOOST_ASIO_CORO_YIELD {
                BOOST_ASIO_HANDLER_LOCATION((__FILE__, __LINE__, "http::async_read"));
                net::post(s_.get_executor(), std::move(self));
            }
        } else {
            do {
                BOOST_ASIO_CORO_YIELD {
                    BOOST_ASIO_HANDLER_LOCATION((__FILE__, __LINE__, "http::async_read"));
                    // 如果没有解析完，继续读
                    async_read_some(s_, b_, p_, std::move(self));
                }
                bytes_transferred_ += bytes_transferred;
            } while (!ec && !Condition{}(p_));
        }
        self.complete(ec, bytes_transferred_);
    }
}

template <class AsyncReadStream, class DynamicBuffer, typename ReadHandler>
BOOST_BEAST_ASYNC_RESULT2(ReadHandler)
async_read(AsyncReadStream& stream, DynamicBuffer& buffer, sl_req_decoder& decoder, ReadHandler&& handler) {
    return net::async_compose<ReadHandler, void(boost::beast::error_code, std::size_t)>(
        read_op<AsyncReadStream, DynamicBuffer, parser_is_done>(stream, buffer, decoder), handler, stream);
}

sl_session::sl_session(tcp::socket&& socket, net::io_context& ioc) : socket_(std::move(socket)), _timer(ioc) {
    this->codec_ = std::make_unique<sl_req_decoder>();
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    this->uuid_ = boost::uuids::to_string(a_uuid);
    this->running_ = false;
}

sl_session::~sl_session() {
    this->close("析构关闭");
}

void sl_session::run() {
    if (this->running_) {
        return;
    }
    // 修改状态在运行中
    this->running_ = true;
    this->start_timer();
    net::dispatch(socket_.get_executor(), beast::bind_front_handler(&sl_session::do_read, this->shared_from_this()));
}

void sl_session::do_read() {
    async_read(socket_, buffer_, *codec_, beast::bind_front_handler(&sl_session::on_read, shared_from_this()));
}

void sl_session::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    // 客户端主动关闭
    if (ec == net::error::eof) {
        this->close(ec.message());
        return;
    }
    if (ec) {
        LOG_ERROR << ec.message();
        // todo 如果这里报错应该直接关闭？
        return;
    }
    // fixme 这里如果什么都不传，会有报错
    boost::ignore_unused(bytes_transferred);
    auto& tp = sys_thread_pool::instance().thread_pool();
    auto self = shared_from_this();
    // 此对象交给 asio 获取，证明已经切包完成
    sl_full_buf full_buf = self->codec_->release();
    // 将解析逻辑和 io 线程分离开
    net::post(*tp, [self, fb = std::move(full_buf)] {
        try {
            auto buffer = self->codec_->protocol_define()->parse_template(fb);
            // 处理完成后交给 asio 回调
            self->do_write(std::move(buffer));
        } catch (std::exception& e) {
            LOG_ERROR << e.what();
            // 写出失败就关闭吧
            std::ostringstream message;
            message << "出现异常关闭，原因 " << e.what();
            self->close(message.str());
        } catch (...) {
            LOG_ERROR << "出现未知异常关闭";
            self->close("出现未知异常关闭");
        }
    });
    this->reset_timer();
}

void sl_session::do_write(std::optional<std::shared_ptr<asio::streambuf>>&& buffer) {
    auto self = shared_from_this();
    auto& buf_ptr = *buffer;
    net::async_write(socket_, *buf_ptr, [self, buf_ptr](boost::system::error_code ec, std::size_t bytes_transferred) {
        if (ec) {
            LOG_ERROR << " 写出失败！" << ec.message();
        }
        // 即使失败，也继续读
        self->do_read();
    });
}

const std::string& sl_session::uuid() {
    return uuid_;
}

void sl_session::close(const std::string& reason) {
    if (!this->socket_.is_open()) {
        return;
    }
    boost::system::error_code ec;
    this->socket_.shutdown(tcp::socket::shutdown_both, ec);
    this->socket_.close(ec);
    this->_timer.cancel();
    LOG_DEBUG << " 关闭连接 " << reason;
}

void sl_session::start_timer() {
    this->reset_timer();
}

void sl_session::reset_timer() {
    const auto& session = conf::sl::instance().get_server().get_session();
    _timer.expires_after(std::chrono::seconds(session.get_reader_idle_time_out()));
    auto self = this->shared_from_this();
    _timer.async_wait([this, self](const boost::system::error_code& ec) {
        if (!ec) {
            // 定时器到期，说明超时未收到消息
            this->close("超时自动关闭");
        }
    });
}
