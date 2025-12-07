#include <iostream>
#include <boost/asio.hpp>

#include "src/sl/sl_basic.hpp"
#include "src/core/server_init.hpp"
#include "src/core/log.hpp"

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

int main(int argc, char* argv[]) {
    logger::init();
    conf::sl &c = conf::sl::instance();
    c.init(argc, argv);
    const auto &server = c.get_server();
    const auto &ip = server.get_listen_ip();
    auto const address = net::ip::make_address(ip);
    auto const threads = std::max<int>(1, 4);
    net::io_context ioc{threads};

    auto port = server.get_port();
    std::make_shared<sl_listener>(ioc, tcp::endpoint{address, port})->run();
    LOG_INFO << "服务端启动 ip " << ip << " 端口 " << port;

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
            [&](beast::error_code const &, int)
            {
                // Stop the `io_context`. This will cause `run()`
                // to return immediately, eventually destroying the
                // `io_context` and all of the sockets in it.
                ioc.stop();
            });

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back(
                [&ioc]
                {
                    ioc.run();
                });
    ioc.run();

    // (If we get here, it means we got a SIGINT or SIGTERM)

    // Block until all the threads exit
    for (auto &t : v)
        t.join();

    return 0;
}
