#include <boost/asio/detached.hpp>
#include <spdlog/spdlog.h>

#include "yaml_ser_deser.hpp"

#include "server_init.hpp"

static void load_cfg(YAML::Node& cfg_node, const std::string& cfg_path) {
    try {
        cfg_node = YAML::LoadFile(cfg_path);
    } catch (const YAML::ParserException& parser) {
        SPDLOG_CRITICAL("配置文件解析错误: {}", parser.what());
        std::exit(EXIT_FAILURE);
    } catch (const YAML::BadFile& bad) {
        SPDLOG_CRITICAL("读取 yaml 配置文件错误: {}", bad.what());
        std::exit(EXIT_FAILURE);
    }
}

namespace conf {

    void sl::init(const po::variables_map& vm) {
        auto path_str = vm.at(CONFIG_FILE_KEY).as<std::string>();
        if (!std::filesystem::exists(path_str)) {
            SPDLOG_CRITICAL("{} 配置文件不存在！", path_str);
            std::exit(EXIT_FAILURE);
        }
        // 读取 YAML 文件
        YAML::Node config;
        load_cfg(config, path_str);
        try {
            this->_server.init(config["server"]);
            SPDLOG_INFO("加载 server 配置成功！");
            this->_callback.init(config["callback"]);
        } catch (std::exception& e) {
            SPDLOG_CRITICAL("读取配置失败！ {}", e.what());
            std::exit(EXIT_FAILURE);
        }
    }

    const server& sl::get_server() const {
        return _server;
    }

    const call_back& sl::get_callback() const {
        return _callback;
    }
} // namespace conf

namespace logger {

    spdlog::level::level_enum init(const po::variables_map& vm) {
        // 设置日志样式
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [tid:%t] %s:%# %v");
        auto path_str = vm.at(CONFIG_FILE_KEY).as<std::string>();
        spdlog::level::level_enum log_level = spdlog::level::info;
        // 配置文件存在，读取配置文件中的配置
        if (std::filesystem::exists(path_str)) {
            try {
                YAML::Node config = YAML::LoadFile(path_str);
                YAML::Node server = config["server"];
                if (server.IsDefined() && server["log"].IsDefined()) {
                    YAML::Node log_node = server["log"];
                    log_level = log_node["level"].as<spdlog::level::level_enum>();
                }
            } catch (const std::runtime_error& e) {
                // 什么都不做，报错就报错吧，确保日志正确的初始化
                SPDLOG_WARN("日志配置报错 {} , 使用默认等级info", e.what());
            }
        }
        spdlog::set_level(log_level);
        return log_level;
    }

} // namespace logger
