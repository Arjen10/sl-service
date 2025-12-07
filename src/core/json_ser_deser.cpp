//
// Created by Arjen on 2025/10/30.
//

#include "json_ser_deser.hpp"

void boost::posix_time::to_json(nlohmann::json &j, const boost::posix_time::ptime &ptime) {
    // 将 'T' 替换为空格，得到 yyyy-mm-dd hh:mm:ss 格式
    std::string formatted_time = boost::posix_time::to_iso_extended_string(ptime);
    // 替换 'T' 为空格
    formatted_time.replace(10, 1, " ");
    j = formatted_time;
}
