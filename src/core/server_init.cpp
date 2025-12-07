#include "server_init.hpp"

namespace conf {

    void sl::init(int argc, char **argv) {
        // 定义命令行选项
        po::options_description desc("参数说明");
        // 默认配置文件位置
        std::filesystem::path def_conf = std::filesystem::current_path() / "config.yaml";
        desc.add_options()
                ("config,c", po::value<std::string>()->default_value(def_conf.string()), "配置文件路径");
        po::variables_map vm;
        try {
            po::store(po::parse_command_line(argc, argv, desc), vm);
            po::notify(vm);
        } catch (const po::error &e) {
            throw e;
        }
        auto path_str = vm.at(CONFIG_FILE_KEY).as<std::string>();
        if (!std::filesystem::exists(path_str)) {
            throw std::runtime_error(path_str + " 配置文件不存在！");
        }
        // 读取 YAML 文件
        YAML::Node config;
        try {
            config = YAML::LoadFile(path_str);
        } catch (const YAML::ParserException &parser) {
            LOG_FATAL << "配置文件解析错误：" << parser.what();
            std::exit(EXIT_FAILURE);
        } catch (const YAML::BadFile &bad) {
            LOG_FATAL << "读取 yaml 配置文件错误：" << bad.what();
            std::exit(EXIT_FAILURE);
        }
        this->_server.init(config["server"]);
        LOG_INFO << "加载 server 配置成功";
        this->_callback.init(config["callback"]);
        LOG_INFO << "加载 callback 配置成功";
    }

    const server &sl::get_server() const {
        return _server;
    }

    const call_back &sl::get_callback() const {
        return _callback;
    }

    void server::init(const YAML::Node &node) {
        this->_listen_ip = node["listen-ip"].as<std::string>("127.0.0.1");
        this->_port = node["port"].as<uint16_t>(4399);
        this->_session.init(node["session"]);
        this->_thread.init(node["thread"]);
    }

    const std::string &server::get_listen_ip() const {
        return _listen_ip;
    }

    uint16_t server::get_port() const {
        return _port;
    }

    const session &server::get_session() const {
        return _session;
    }

    const thread &server::get_thread() const {
        return _thread;
    }

    void session::init(const YAML::Node &node) {
        this->_max_buffer = node["max-buffer"].as<std::size_t>(2028);
        this->_reader_idle_time_out = node["reader-idle-time-out"].as<std::size_t>(900);
    }

    size_t session::get_max_buffer() const {
        return _max_buffer;
    }

    size_t session::get_reader_idle_time_out() const {
        return _reader_idle_time_out;
    }

    void thread::init(const YAML::Node &node) {
        this->_asio_ioc = node["asio-ioc"].as<std::size_t>(2);
        this->_pool = node["pool"].as<std::size_t>(2);
    }

    void call_back::init(const YAML::Node &node) {
        if (!node.IsDefined()) {
            LOG_WARN << "回调没有配置";
            this->_enable = false;
            return;
        }
        this->_enable = node.as<bool>(false);
        // 如果不开启，直接返回
        if (!_enable) {
            return;
        }
        this->_http_conf.init(node["http"]);
        this->_mqtt.init(node["mqtt"]);
    }

    const std::optional<http> &call_back::get_http_conf() const {
        if (!this->_enable) {
            return std::nullopt;
        }
        return _http_conf;
    }

    const std::optional<mqtt> &call_back::get_mqtt_conf() const {
        if (!this->_enable) {
            return std::nullopt;
        }
        return _mqtt;
    }

    void http::init(const YAML::Node &node) {
        if (node.IsNull()) {
            return;
        }
        this->_url = node["url"].as<std::string>();
        this->_connect_time_out = node["connect-time-out"].as<uint32_t>();
    }

    const std::string &http::get_url() const {
        return _url;
    }

    const uint32_t http::get_connect_time_out() const {
        return _connect_time_out;
    }

    void mqtt::init(const YAML::Node &node) {
        if (node.IsNull()) {
            return;
        }
        this->_broker_host = node["broker-host"].as<std::string>();
        this->_broker_port = node["broker-port"].as<std::uint16_t>();
        this->_client_id = node["client-id"].as<std::string>();
        this->_topic_prefix = node["topic-prefix"].as<std::string>();
        auto qos = node["qos"].as<std::uint8_t>();
        if (qos == 0 || qos == 1 || qos == 2) {
            this->_qos = static_cast<boost::mqtt5::qos_e>(qos);
        } else {
            LOG_ERROR << "错误的 qos " << qos;
            std::exit(EXIT_FAILURE);
        }
    }

    const std::string &mqtt::get_broker_host() const {
        return _broker_host;
    }

    const uint16_t mqtt::get_broker_port() const {
        return _broker_port;
    }

    const std::string mqtt::get_client_id() const {
        return _client_id;
    }

    const std::string mqtt::get_topic_prefix() const {
        return _topic_prefix;
    }

}

namespace mqtt5 {

    void client::init(boost::asio::io_context &ioc) {
        if (_is_init) {
            return;
        }
        _client = std::make_shared<mqtt_client_type>(
                ioc, std::monostate{}, boost::mqtt5::logger(boost::mqtt5::log_level::info)
        );
        auto &cfg = conf::sl::instance().get_callback().get_mqtt_conf();
        // 设置 broker、clientId、持久会话等
        _client->brokers(cfg->get_broker_host(), cfg->get_broker_port())
                .credentials(cfg->get_client_id())
                .connect_property(boost::mqtt5::prop::session_expiry_interval, 60);

        // 开始异步运行（保持连接）
        _client->async_run(
                [self = shared_from_this()](boost::mqtt5::error_code ec) {
                    if (ec) {
                        LOG_ERROR << "MQTT run error: " << ec.message();
                        // todo 这里可加自动重连逻辑
                    }
                }
        );
        _is_init = true;
    }

    void client::publish_async(const std::string &stcd, const std::string &message) {
        auto &cfg = conf::sl::instance().get_callback().get_mqtt_conf();
        auto self = this->shared_from_this();
        std::string topic = cfg->get_topic_prefix() + "/" + stcd;
        self->_client->async_publish<boost::mqtt5::qos_e::at_most_once>(
                topic,
                message,
                boost::mqtt5::retain_e::yes,
                boost::mqtt5::publish_props{},
                [self, topic, message](boost::mqtt5::error_code ec) {
                    if (!ec) {
                        LOG_DEBUG << "[MQTT publish OK] topic= " << topic;
                        return;
                    }
                    LOG_ERROR << "[MQTT publish FAILED] topic= " << topic
                              << " message= " << message
                              << " ec=" << ec.message();
                    return;
                }
        );
    }

}

namespace logger {

    void init() {
        // 初始化日志输出到控制台
        logging::add_console_log(
                std::clog,
                boost::log::keywords::format = (
                        expr::stream
                                << "[" << expr::attr<boost::posix_time::ptime>("TimeStamp") << "] "
                                << "<" << logging::trivial::severity << "> "
                                << "[Thread "
                                << expr::attr<boost::log::attributes::current_thread_id::value_type>("ThreadID") << "] "
                                << expr::smessage
                )
        );
        // 添加时间戳和线程ID等属性
        logging::add_common_attributes();
        BOOST_LOG_TRIVIAL(info) << "初始化日志成功！";
    }

}
