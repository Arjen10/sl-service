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
        try {
            this->_server.init(config["server"]);
            LOG_INFO << "加载 server 配置成功";
            this->_callback.init(config["callback"]);
        } catch (std::exception &e) {
            LOG_ERROR << "读取配置失败！ " << e.what();
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
    }

}
