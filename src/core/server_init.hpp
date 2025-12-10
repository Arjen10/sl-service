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


#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <optional>
#include <cstdint>

#include "log.hpp"
#include "singleton_template.hpp"
#include "server_config.hpp"

#define CONFIG_FILE_KEY "config"

/**
 * 配置文件相关
 */
namespace conf {

    namespace po = boost::program_options;

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

namespace logger {

    namespace logging = boost::log;
    namespace expr = boost::log::expressions;
    namespace keywords = boost::log::keywords;

    void init();

}


#endif //SL_SERVICE_SERVER_INIT_HPP
