//
// Created by Arjen on 2025/12/8.
//

#include <iostream>

#include <boost/version.hpp>
#include <boost/program_options.hpp>
#include <boost/log/sources/severity_logger.hpp>

#include "app.hpp"
#include "../core/mqtt5_client.hpp"
#include "../core/sys_thread_pool.hpp"

app::app(int argc, char* argv[]) : _argc(argc), _argv(argv) {
    this->_ioc = nullptr;
}

app::~app() {
    if (this->_ioc) {
        this->_ioc->stop();
    }
    for (auto& t : _threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

int app::run() {
    this->argc_to_vm();
    this->setup_logger();
    LOG_INFO << "Boost Version: " << BOOST_VERSION / 100000 << "." << BOOST_VERSION / 100 % 1000 << "."
             << BOOST_VERSION % 100;
    this->load_config();
    // ioc 必须在配置文件初始化完成后立即初始
    this->ioc_init();
    this->start_listener();
    this->mqtt5_init();
    this->start_sys_threads_pool();
    // 先启动子线程
    spawn_io_threads();
    // 主线程也 run
    _ioc->run();
    return 0;
}

void app::argc_to_vm() {
    // 定义命令行选项
    po::options_description desc("参数说明");
    // 默认配置文件位置
    std::filesystem::path def_conf = std::filesystem::current_path() / "config.yaml";
    desc.add_options()("config,c", po::value<std::string>()->default_value(def_conf.string()), "配置文件路径");
    try {
        po::store(po::parse_command_line(this->_argc, this->_argv, desc), _vm);
        po::notify(_vm);
    } catch (const po::error& e) {
        std::cerr << "读取运行参数错误" << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void app::setup_logger() {
    auto log_level = logger::init(this->_vm);
    LOG_INFO << "日志初始化成功，当前日志等级 " << boost::log::trivial::to_string(log_level);
}

void app::load_config() {
    conf::sl::instance().init(this->_vm);
}

void app::ioc_init() {
    auto& s_cfg = conf::sl::instance().get_server();
    // 最少一个线程
    auto thread_num = std::max<int>(1, s_cfg.get_thread().asio_ioc());
    this->_ioc = std::make_unique<net::io_context>(thread_num);
}

void app::mqtt5_init() {
    auto& cfg_opt = conf::sl::instance().get_callback().get_mqtt_conf();
    if (!cfg_opt.has_value()) {
        return;
    }
    try {
        mqtt5_client::instance().init(*this->_ioc);
        auto& mqtt5_cfg = cfg_opt.value();
    } catch (std::exception& e) {
        LOG_FATAL << "初始化 MQTT5 客户端失败！" << e.what();
        std::exit(EXIT_FAILURE);
    }
}

void app::start_listener() {
    auto& s_cfg = conf::sl::instance().get_server();
    const auto& ip = s_cfg.get_listen_ip();
    auto const address = net::ip::make_address(ip);
    auto port = s_cfg.get_port();
    _listener = std::make_shared<sl_listener>(*this->_ioc, tcp::endpoint{address, port});
    this->_listener->run();
    LOG_INFO << "开始监听 ip: " << ip << " prot: " << port;
    // 停止信号
    this->_signals = std::make_unique<net::signal_set>(*this->_ioc, SIGINT, SIGTERM);
    this->_signals->async_wait([&](beast::error_code const&, int) { this->_ioc->stop(); });
}

void app::start_sys_threads_pool() {
    auto& s_cfg = conf::sl::instance().get_server();
    auto sys_threads = s_cfg.get_thread().pool();
    sys_thread_pool::instance().init(sys_threads);
    LOG_INFO << "初始化业务线程池成功，线程数量 " << sys_threads;
}

void app::spawn_io_threads() {
    auto& s_cfg = conf::sl::instance().get_server();
    auto asio_ioc = s_cfg.get_thread().asio_ioc();
    // 线程数量排除掉主线程
    _threads.reserve(asio_ioc - 1);
    for (std::size_t i = 0; i < asio_ioc - 1; ++i) {
        _threads.emplace_back([this] { _ioc->run(); });
    }
    LOG_INFO << "初始化 asio ioc 成功，线程数量 " << asio_ioc;
}
