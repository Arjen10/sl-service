//
// Created by Arjen on 2025/12/9.
//

#ifndef SL_SERVICE_YAML_SER_DESER_HPP
#define SL_SERVICE_YAML_SER_DESER_HPP

#include <yaml-cpp/node/node.h>
#include <boost/algorithm/string.hpp>
#include <spdlog/common.h>

#include "callback_method.hpp"

namespace YAML {
    template <> struct convert<method> {
        static bool decode(const Node& node, method& rhs) {
            static const std::unordered_map<std::string, method> m = {
                {"http", method::http_callback},
                {"mqtt", method::mqtt_callback},
            };
            auto s = node.as<std::string>();
            auto it = m.find(s);
            if (it == m.end()) {
                return method::unknown;
            }
            rhs = it->second;
            return true;
        }
    };

    template <> struct convert<spdlog::level::level_enum> {
        static bool decode(const Node& node, spdlog::level::level_enum& rhs) {
            static const std::unordered_map<std::string, spdlog::level::level_enum> m = {
                {"trace", spdlog::level::trace},
                {"debug", spdlog::level::debug},
                {"info", spdlog::level::info},
                {"warn", spdlog::level::warn},
                {"err", spdlog::level::err},
                {"critical", spdlog::level::critical}
            };
            auto key = node.as<std::string>();
            boost::to_lower(key);
            auto it = m.find(key);
            if (it == m.end()) {
                // 默认 info
                return spdlog::level::info;
            }
            rhs = it->second;
            return true;
        }
    };

} // namespace YAML

#endif // SL_SERVICE_YAML_SER_DESER_HPP
