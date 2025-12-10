//
// Created by Arjen on 2025/12/8.
//

#ifndef SL_SERVICE_APP_HPP
#define SL_SERVICE_APP_HPP

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>

#include <thread>
#include <memory>

#include "sl_listener.hpp"

namespace net = boost::asio;

class app {
public:
    explicit app(int argc, char* argv[]);
    ~app();
    int run();
private:
    void setup_logger();
    void load_config();
    void ioc_init();
    void mqtt5_init();
    void start_listener();
    void spawn_io_threads();
private:
    int _argc;
    char** _argv;
    std::unique_ptr<net::io_context> _ioc;
    std::vector<std::thread> _threads;
    std::shared_ptr<sl_listener> _listener;
    std::unique_ptr<net::signal_set> _signals;
};

#endif //SL_SERVICE_APP_HPP
