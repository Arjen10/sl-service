#include "yaml_ser_deser.hpp"

#include "server_init.hpp"

static void load_cfg(YAML::Node &cfg_node, const std::string &cfg_path) {
    try {
        cfg_node = YAML::LoadFile(cfg_path);
    } catch (const YAML::ParserException &parser) {
        LOG_FATAL << "配置文件解析错误：" << parser.what();
        std::exit(EXIT_FAILURE);
    } catch (const YAML::BadFile &bad) {
        LOG_FATAL << "读取 yaml 配置文件错误：" << bad.what();
        std::exit(EXIT_FAILURE);
    }
}

namespace conf {

    void sl::init(const po::variables_map &vm) {
        auto path_str = vm.at(CONFIG_FILE_KEY).as<std::string>();
        if (!std::filesystem::exists(path_str)) {
            LOG_FATAL << path_str << " 配置文件不存在！";
            std::exit(EXIT_FAILURE);
        }
        // 读取 YAML 文件
        YAML::Node config;
        load_cfg(config, path_str);
        try {
            this->_server.init(config["server"]);
            LOG_INFO << "加载 server 配置成功";
            this->_callback.init(config["callback"]);
        } catch (std::exception &e) {
            LOG_FATAL << "读取配置失败！ " << e.what();
            std::exit(EXIT_FAILURE);
        }
    }

    const server &sl::get_server() const {
        return _server;
    }

    const call_back &sl::get_callback() const {
        return _callback;
    }
}

namespace logger {

    boost::log::trivial::severity_level init(const po::variables_map &vm) {
        auto path_str = vm.at(CONFIG_FILE_KEY).as<std::string>();
        boost::log::trivial::severity_level log_level = boost::log::trivial::severity_level::info;
        // 配置文件存在，读取配置文件中的配置
        if (std::filesystem::exists(path_str)) {
            try {
                YAML::Node config = YAML::LoadFile(path_str);
                YAML::Node server = config["server"];
                if (server.IsDefined() && server["log"].IsDefined()) {
                    YAML::Node log_node = server["log"];
                    log_level = log_node["level"].as<boost::log::trivial::severity_level>();
                }
            } catch (const std::runtime_error &ignore) {
                // 什么都不做，报错就报错吧，确保日志正确的初始化
            }
        }
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
                ),
                boost::log::keywords::filter = (logging::trivial::severity >= log_level)
        );
        // 添加时间戳和线程ID等属性
        logging::add_common_attributes();
        return log_level;
    }

}
