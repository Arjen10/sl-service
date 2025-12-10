//
// Created by Arjen on 2025/12/7.
//

#ifndef SL_SERVICE_SERVER_CONFIG_HPP
#define SL_SERVICE_SERVER_CONFIG_HPP

#include <boost/program_options.hpp>
#include <boost/mqtt5/types.hpp>
#include <yaml-cpp/yaml.h>

#include "log.hpp"
#include "callback_method.hpp"

namespace po = boost::program_options;

class sl;

class server;

class call_back;

class http_cfg {
    friend call_back;
private:
    std::string _url;
    uint32_t _connect_time_out;

public:
    void init(const YAML::Node &node);

    const std::string &get_url() const;

    const uint32_t get_connect_time_out() const;
};

class mqtt {
    friend call_back;
private:
    std::string _broker_host;
    uint16_t _broker_port;
    std::string _client_id;
    std::string _username;
    std::string _password;
    std::string _topic_prefix;
    boost::mqtt5::qos_e _qos;

public:
    void init(const YAML::Node &node);

    const std::string &get_broker_host() const;

    const uint16_t get_broker_port() const;

    const std::string get_client_id() const;

    const std::string &get_username() const;

    const std::string &get_password() const;

    const std::string &get_topic_prefix() const;

    const boost::mqtt5::qos_e &get_qos() const;
};

class call_back {
    friend sl;
private:
    bool _enable;
    std::optional<method> _method;
    std::optional<http_cfg> _http_conf;
    std::optional<mqtt> _mqtt;

public:
    void init(const YAML::Node &node);
    bool enable() const;
    const std::optional<method> &get_callback_method() const;
    const std::optional<http_cfg> &get_http_conf() const;
    const std::optional<mqtt> &get_mqtt_conf() const;
};

class session {
    friend server;
private:
    std::size_t _max_buffer;
    std::size_t _reader_idle_time_out;

    void init(const YAML::Node &node);

public:
    size_t get_max_buffer() const;

    size_t get_reader_idle_time_out() const;
};

class thread {
    friend server;
private:
    std::size_t _asio_ioc;
    std::size_t _pool;
    void init(const YAML::Node &node);

public:
    size_t asio_ioc() const;
    size_t pool() const;
};

class server {
    friend sl;
private:
    std::string _listen_ip;
    uint16_t _port;
    session _session;
    thread _thread;

public:
    void init(const YAML::Node &node);

    const std::string &get_listen_ip() const;

    uint16_t get_port() const;

    const session &get_session() const;

    const thread &get_thread() const;
};


#endif //SL_SERVICE_SERVER_CONFIG_HPP
