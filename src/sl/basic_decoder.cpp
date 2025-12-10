//
// Created by Arjen on 2024/12/30.
//

#include <utility>

#include "../core/mqtt5_client.hpp"
#include "../core/mime.hpp"
#include "basic_decoder.hpp"

namespace callback {

    void union_json(nlohmann::json &ret,
                    const nlohmann::json &h_j,
                    const nlohmann::json &c_j,
                    const nlohmann::json &e_j) {
        ret["header"] = h_j;
        ret["content"] = c_j;
        ret["end"] = e_j;
    }

    const std::unordered_map<method, func> _map = {

            {method::http_callback, [](const std::string &stcd, const nlohmann::json &h_j,
                                       const nlohmann::json &c_j, const nlohmann::json &e_j) {
                const auto &http_conf = conf::sl::instance().get_callback().get_http_conf();
                if (!http_conf.has_value()) {
                    LOG_WARN << " http 回调配置为空！";
                    return;
                }
                nlohmann::json j = nlohmann::json::object();
                union_json(j, h_j, c_j, e_j);
                auto ms = chrono::seconds(http_conf.value().get_connect_time_out());
                try {
                    auto resp = cpr::Post(
                            cpr::Url{http_conf.value().get_url()},
                            cpr::Header{{beast::http::to_string(beast::http::field::content_type), APPLICATION_JSON}},
                            cpr::Body(j.dump()),
                            cpr::ConnectTimeout{chrono::duration_cast<chrono::milliseconds>(ms)}
                    );
                    LOG_DEBUG << " http 回调接口状态 " << resp.status_code;
                    if (resp.status_code != 200) {
                        LOG_ERROR << resp.reason;
                    }
                } catch (std::exception &e) {
                    LOG_ERROR << "HTTP回调异常！" << e.what();
                }
            }},

            {method::mqtt_callback, [](const std::string &stcd, const nlohmann::json &h_j,
                                       const nlohmann::json &c_j, const nlohmann::json &e_j) {
                const auto &mqtt_cfg = conf::sl::instance().get_callback().get_mqtt_conf();
                if (!mqtt_cfg.has_value()) {
                    LOG_WARN << " mqtt 回调配置为空！";
                    return;
                }
                nlohmann::json j = nlohmann::json::object();
                try {
                    union_json(j, h_j, c_j, e_j);
                    mqtt5_client::instance().publish_async(stcd, j.dump());
                } catch (std::exception &e) {
                    LOG_ERROR << "MQTT回调异常 " << e.what();
                }
            }},
    };

}

raw_hex_value::raw_hex_value(byte_buf_reader &reader, uint16_t length,
                             std::string note, std::string v)
        : _note(std::move(note)), _length(length), _v(std::move(v)) {
    reader.read_hex_str(this->_raw_hex, length);
}

raw_hex_value::raw_hex_value(byte_buf_reader &reader, uint16_t length,
                             std::string note, const nlohmann::json &v)
        : _note(std::move(note)), _length(length), _v(v.get<std::string>()) {
    reader.read_hex_str(this->_raw_hex, length);
}

raw_hex_value::raw_hex_value(byte_buf_reader &reader, uint16_t length,
                             std::string note)
        : _note(std::move(note)), _length(length) {
    reader.read_hex_str(this->_raw_hex, length);
    _v = this->_raw_hex;
}

void raw_hex_value::update(byte_buf_reader &reader, uint16_t length, const std::string &v) {
    std::string temp_str;
    reader.read_hex_str(temp_str, length);
    this->_raw_hex.append(temp_str);
    this->_length += length;
    this->_v = v;
}

void to_json(nlohmann::json &j, const raw_hex_value &value) {
    JSON_FIELD_REF(j, value, _raw_hex);
    JSON_FIELD_REF(j, value, _length);
    JSON_FIELD_REF(j, value, _note);
    JSON_FIELD_REF(j, value, _v);
}
