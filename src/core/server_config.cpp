//
// Created by Arjen on 2025/12/7.
//

#include <filesystem>

#include "../core/yaml_ser_deser.hpp"

#include "server_config.hpp"

namespace fs = std::filesystem;

void server::init(const YAML::Node& node) {
    // https://stackoverflow.com/a/25518538 docker 环境检查
    this->_listen_ip = node["listen-ip"].as<std::string>("127.0.0.1");
    bool in_docker = fs::exists("/.dockerenv") || fs::exists("/proc/1/cgroup");
    if (!in_docker && this->_listen_ip == "127.0.0.1") {
        LOG_WARN << "检测到容器运行环境，当前监听地址为 127.0.0.1。"
                    "在多容器场景下，该配置可能导致服务无法被其他容器访问。"
                    "如需对外或跨容器访问，建议使用 0.0.0.0。";
    }
    this->_port = node["port"].as<uint16_t>(4399);
    this->_session.init(node["session"]);
    this->_thread.init(node["thread"]);
    this->_boost_log.init(node["log"]);
}

const std::string& server::get_listen_ip() const {
    return _listen_ip;
}

uint16_t server::get_port() const {
    return _port;
}

const session& server::get_session() const {
    return _session;
}

const thread& server::get_thread() const {
    return _thread;
}

const boost_log& server::get_log() const {
    return _boost_log;
}

void session::init(const YAML::Node& node) {
    this->_max_buffer = node["max-buffer"].as<std::size_t>(2028);
    this->_reader_idle_time_out = node["reader-idle-time-out"].as<std::size_t>(900);
}

size_t session::get_max_buffer() const {
    return _max_buffer;
}

size_t session::get_reader_idle_time_out() const {
    return _reader_idle_time_out;
}

void thread::init(const YAML::Node& node) {
    auto asio_ioc_v = node["asio-ioc"].as<std::uint16_t>(1);
    this->_asio_ioc = std::max(asio_ioc_v, std::uint16_t{1});
    auto pool_v = node["pool"].as<std::uint16_t>(1);
    this->_pool = std::max(pool_v, std::uint16_t{1});
}

std::uint16_t thread::asio_ioc() const {
    return _asio_ioc;
}

std::uint16_t thread::pool() const {
    return _pool;
}

void call_back::init(const YAML::Node& node) {
    if (!node.IsDefined()) {
        LOG_WARN << "回调没有配置";
        this->_enable = false;
        return;
    }
    auto e_node = node["enable"];
    this->_enable = e_node.as<bool>(false);
    // 如果不开启，直接返回
    if (!_enable) {
        LOG_INFO << "当前 callback 状态为：无需回调";
        return;
    }
    auto mn = node["method"];
    if (!mn.IsDefined()) {
        LOG_FATAL << "开启了回调，但 callback.method 未配置！";
        std::exit(EXIT_FAILURE);
    }
    switch (mn.as<method>()) {
    case method::http_callback: {
        auto http_node = node["http"];
        this->_http_conf.emplace();
        auto& cfg = this->_http_conf.value();
        cfg.init(http_node);
        this->_method = method::http_callback;
        LOG_INFO << "当前 callback 状态为：http 回调，api 接口地址：" << cfg._url;
        return;
    }
    case method::mqtt_callback: {
        auto mqtt_node = node["mqtt"];
        this->_mqtt.emplace();
        auto& cfg = this->_mqtt.value();
        cfg.init(mqtt_node);
        this->_method = method::mqtt_callback;
        LOG_INFO << "当前 callback 状态为：mqtt 回调，mqtt broker 地址：" << cfg._broker_host << " 端口："
                 << cfg._broker_port;
        return;
    }
    default: {
        LOG_FATAL << "错误的 callback.method 配置 " << mn.as<std::string>();
        std::exit(EXIT_FAILURE);
    }
    }
}

bool call_back::enable() const {
    return _enable;
}

const std::optional<method>& call_back::get_callback_method() const {
    return _method;
}

const std::optional<http_cfg>& call_back::get_http_conf() const {
    return _http_conf;
}

const std::optional<mqtt>& call_back::get_mqtt_conf() const {
    return _mqtt;
}

void http_cfg::init(const YAML::Node& node) {
    if (node.IsNull()) {
        return;
    }
    this->_url = node["url"].as<std::string>();
    this->_connect_time_out = node["connect-time-out"].as<uint32_t>();
}

const std::string& http_cfg::get_url() const {
    return _url;
}

const uint32_t http_cfg::get_connect_time_out() const {
    return _connect_time_out;
}

void mqtt::init(const YAML::Node& node) {
    if (node.IsNull()) {
        return;
    }
    this->_broker_host = node["broker-host"].as<std::string>();
    this->_broker_port = node["broker-port"].as<std::uint16_t>();
    this->_client_id = node["client-id"].as<std::string>();
    auto un = node["username"];
    this->_username = un.IsNull() ? "" : un.as<std::string>();
    // 密码默认为空
    auto pwd = node["password"];
    this->_password = pwd.IsNull() ? "" : pwd.as<std::string>();
    this->_topic_prefix = node["topic-prefix"].as<std::string>();
    auto qos = node["qos"].as<std::uint8_t>();
    if (qos == 0 || qos == 1 || qos == 2) {
        this->_qos = static_cast<boost::mqtt5::qos_e>(qos);
    } else {
        LOG_FATAL << "错误的 qos " << qos;
        std::exit(EXIT_FAILURE);
    }
}

const std::string& mqtt::get_broker_host() const {
    return _broker_host;
}

const uint16_t mqtt::get_broker_port() const {
    return _broker_port;
}

const std::string mqtt::get_client_id() const {
    return _client_id;
}

const std::string& mqtt::get_username() const {
    return _username;
}

const std::string& mqtt::get_password() const {
    return _password;
}

const std::string& mqtt::get_topic_prefix() const {
    return _topic_prefix;
}

const boost::mqtt5::qos_e& mqtt::get_qos() const {
    return _qos;
}

void boost_log::init(const YAML::Node& node) {
    if (node.IsNull()) {
        this->_level = boost::log::trivial::info;
        return;
    }
    auto ln = node["level"];
    if (!ln.IsDefined()) {
        // 默认 info 就好
        this->_level = boost::log::trivial::info;
        return;
    }
    this->_level = ln.as<boost::log::trivial::severity_level>();
}

boost::log::trivial::severity_level boost_log::level() {
    return this->_level;
}
