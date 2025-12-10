//
// Created by Arjen on 2025/12/8.
//

#include <boost/version.hpp>

#include "app.hpp"
#include "../core/mqtt5_client.hpp"

app::app(int argc, char *argv[])
        : _argc(argc), _argv(argv) {
    this->_ioc = nullptr;
}

app::~app() {
    if (this->_ioc) {
        this->_ioc->stop();
    }
    for (auto &t: _threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

int app::run() {
    this->setup_logger();
    LOG_INFO << "Boost version: "
             << BOOST_VERSION / 100000 << "."
             << BOOST_VERSION / 100 % 1000 << "."
             << BOOST_VERSION % 100;
    this->load_config();
    // ioc 必须在配置文件初始化完成后立即初始
    this->ioc_init();
    this->start_listener();
    this->mqtt5_init();
    // 先启动子线程
    spawn_io_threads();
    // 主线程也 run
    _ioc->run();
    return 0;
}

void app::setup_logger() {
    logger::init();
    LOG_INFO << "日志初始化成功";
}

void app::load_config() {
    conf::sl::instance().init(this->_argc, this->_argv);
}

void app::ioc_init() {
    auto &s_cfg = conf::sl::instance().get_server();
    // 最少一个线程
    auto thread_num = std::max<int>(1, s_cfg.get_thread().asio_ioc());
    this->_ioc = std::make_unique<net::io_context>(thread_num);
}

void app::mqtt5_init() {
    auto &cfg_opt = conf::sl::instance().get_callback().get_mqtt_conf();
    if (!cfg_opt.has_value()) {
        return;
    }
    try {
        mqtt5_client::instance().init(*this->_ioc);
        auto &mqtt5_cfg = cfg_opt.value();
    } catch (std::exception &e) {
        LOG_ERROR << "初始化 MQTT5 客户端失败！" << e.what();
        std::exit(EXIT_FAILURE);
    }

}

void app::start_listener() {
    auto &s_cfg = conf::sl::instance().get_server();
    const auto &ip = s_cfg.get_listen_ip();
    auto const address = net::ip::make_address(ip);
    auto port = s_cfg.get_port();
    _listener = std::make_shared<sl_listener>(*this->_ioc, tcp::endpoint{address, port});
    this->_listener->run();
    LOG_INFO << "开始监听 ip: " << ip << " prot: " << port;
    // 停止信号
    this->_signals = std::make_unique<net::signal_set>(*this->_ioc, SIGINT, SIGTERM);
    this->_signals->async_wait(
            [&](beast::error_code const &, int) {
                this->_ioc->stop();
            });
}

void app::spawn_io_threads() {
    auto &s_cfg = conf::sl::instance().get_server();
    auto asio_ioc = s_cfg.get_thread().asio_ioc();
    // 线程数量排除掉主线程
    _threads.reserve(asio_ioc - 1);
    for (std::size_t i = 0; i < asio_ioc - 1; ++i) {
        _threads.emplace_back([this] { _ioc->run(); });
    }
}
