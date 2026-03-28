#ifndef SL_SERVICE_SERVER_INIT_HPP
#define SL_SERVICE_SERVER_INIT_HPP

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>
#include <yaml-cpp/yaml.h>
#include <spdlog/common.h>

#include <filesystem>
#include <optional>
#include <cstdint>

#include "singleton_template.hpp"
#include "server_config.hpp"

#define CONFIG_FILE_KEY "config"

/**
 * 配置文件相关
 */
namespace conf {

    namespace po = boost::program_options;

    class sl : public singleton_template<sl> {
      private:
        server _server;
        call_back _callback;
        sl() = default;
        friend class singleton_template<sl>;

      public:
        void init(const po::variables_map& vm);
        const server& get_server() const;
        const call_back& get_callback() const;
    };

} // namespace conf

namespace logger {

    namespace po = boost::program_options;

    spdlog::level::level_enum init(const po::variables_map& vm);

} // namespace logger

#endif // SL_SERVICE_SERVER_INIT_HPP
