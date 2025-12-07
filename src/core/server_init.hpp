#ifndef SL_SERVICE_SERVER_INIT_HPP
#define SL_SERVICE_SERVER_INIT_HPP

#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>
#include <boost/mqtt5/logger.hpp>
#include <boost/mqtt5/mqtt_client.hpp>
#include <boost/mqtt5/types.hpp>

#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <optional>
#include <cstdint>

#include "log.hpp"
#include "singleton_template.hpp"

#define CONFIG_FILE_KEY "config"

/**
 * 配置文件相关
 */
namespace conf {

    namespace po = boost::program_options;

    class sl;
    class server;
    class call_back;

    class http {
        friend call_back;
    private:
        std::string _url;
        uint32_t _connect_time_out;
        void init(const YAML::Node &node);
    public:
        const std::string &get_url() const;
        const uint32_t get_connect_time_out() const;
    };

    class mqtt {
        friend call_back;
    private:
        std::string _broker_host;
        uint16_t _broker_port;
        std::string _client_id;
        std::string _topic_prefix;
        boost::mqtt5::qos_e _qos;
        void init(const YAML::Node &node);
    public:
        const std::string &get_broker_host() const;
        const uint16_t get_broker_port() const;
        const std::string get_client_id() const;
        const std::string get_topic_prefix() const;
    };

    class call_back {
        friend sl;
    private:
        bool _enable;
        uint8_t _method{};
        http _http_conf;
        mqtt _mqtt;
        void init(const YAML::Node &node);

    public:
        const std::optional<http> &get_http_conf() const;
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
    };

    class server {
        friend sl;
    private:
        std::string _listen_ip;
        uint16_t _port;
        session _session;
        thread _thread;
        void init(const YAML::Node &node);

    public:
        const std::string &get_listen_ip() const;
        uint16_t get_port() const;
        const session &get_session() const;
        const thread &get_thread() const;
    };

    class sl: public singleton_template<sl> {
    private:
        server _server;
        call_back _callback;
        sl() = default;
        friend class singleton_template<sl>;
    public:
        void init(int argc, char *argv[]);
        const server &get_server() const;
        const call_back &get_callback() const;
    };

}

namespace mqtt5 {

    using mqtt_client_type = boost::mqtt5::mqtt_client<
            boost::asio::ip::tcp::socket,
            std::monostate,
            boost::mqtt5::logger
    >;

    class client: public std::enable_shared_from_this<client> {
    private:
        std::shared_ptr<mqtt_client_type> _client;
        bool _is_init;
        client() = default;
    public:
        void init(boost::asio::io_context &ioc);
        void publish_async(const std::string &stcd, const std::string &message);
    };

}

namespace logger {

    namespace logging = boost::log;
    namespace expr = boost::log::expressions;
    namespace keywords = boost::log::keywords;

    void init();

}


#endif //SL_SERVICE_SERVER_INIT_HPP
