//
// Created by Arjen on 2025/10/30.
//

#ifndef SL_SERVICE_SL_SESSION_HPP
#define SL_SERVICE_SL_SESSION_HPP

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/beast/http/error.hpp>
#include <memory>

#include "sl_req_decoder.hpp"
#include "../core/boost_thread_pool.hpp"
#include "../core/server_init.hpp"

namespace http = beast::http;
namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

struct parser_is_done
{
    bool operator()(sl_req_decoder const &p) const;
};

template<class AsyncReadStream, class DynamicBuffer>
class read_some_op: net::coroutine
{
    AsyncReadStream &s_;
    DynamicBuffer &b_;
    sl_req_decoder &p_;
    std::size_t bytes_transferred_;
    bool cont_;
public:
    read_some_op(AsyncReadStream &s, DynamicBuffer &b, sl_req_decoder &p);
    template<class Self>
    void operator()(Self &self, beast::error_code ec = {}, std::size_t bytes_transferred = 0);
};

template<
        class AsyncReadStream,
        class DynamicBuffer,
        BOOST_BEAST_ASYNC_TPARAM2 ReadHandler>
BOOST_BEAST_ASYNC_RESULT2(ReadHandler)
async_read_some(AsyncReadStream &stream, DynamicBuffer &buffer,
                sl_req_decoder &parser, ReadHandler &&handler);

template<class Stream, class DynamicBuffer, class Condition>
class read_op: net::coroutine
{
    Stream &s_;
    DynamicBuffer &b_;
    sl_req_decoder &p_;
    std::size_t bytes_transferred_;

public:
    read_op(Stream &s, DynamicBuffer &b, sl_req_decoder &p);
    template<class Self>
    void operator()(Self &self, beast::error_code ec = {}, std::size_t bytes_transferred = 0);
};

class sl_session: public std::enable_shared_from_this<sl_session>
{

public:

    explicit sl_session(tcp::socket &&socket, net::io_context &ioc);

    ~sl_session();

private:

    bool running_;

    tcp::socket socket_;

    std::string uuid_;

    std::unique_ptr<sl_req_decoder> codec_;

    beast::flat_buffer buffer_;

    /**
     * 超时管理器
     */
    boost::asio::steady_timer _timer;

    /**
     * 开启超时计时器
     */
    void start_timer();

    /**
     * 重置超时计时器
     */
    void reset_timer();

public:

    /**
     * session 开始监听
     */
    void run();

    void do_read();

    void on_read(beast::error_code ec, std::size_t bytes_transferred);

    /**
     * 此函数为双工，请求响应模式
     * @param buffer
     */
    void do_write(std::optional<std::shared_ptr<asio::streambuf>> &&buffer);

    void close(const std::string &reason);

    const std::string& uuid();

};


#endif //SL_SERVICE_SL_SESSION_HPP
