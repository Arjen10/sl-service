//
// Created by Arjen on 2025/12/9.
//

#ifndef SL_SERVICE_YAML_SER_DESER_HPP
#define SL_SERVICE_YAML_SER_DESER_HPP

#include <yaml-cpp/node/node.h>
#include <boost/log/trivial.hpp>
#include <boost/algorithm/string.hpp>

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

    template <> struct convert<boost::log::trivial::severity_level> {
        static bool decode(const Node& node, boost::log::trivial::severity_level& rhs) {
            static const std::unordered_map<std::string, boost::log::trivial::severity_level> m = {
                {"trace", boost::log::trivial::severity_level::trace},
                {"debug", boost::log::trivial::severity_level::debug},
                {"info", boost::log::trivial::severity_level::info},
                {"warning", boost::log::trivial::severity_level::warning},
                {"error", boost::log::trivial::severity_level::error},
                {"fatal", boost::log::trivial::severity_level::fatal},
            };
            auto key = node.as<std::string>();
            boost::to_lower(key);
            auto it = m.find(key);
            if (it == m.end()) {
                // 默认 info
                return boost::log::trivial::severity_level::info;
            }
            rhs = it->second;
            return true;
        }
    };

} // namespace YAML

#endif // SL_SERVICE_YAML_SER_DESER_HPP
