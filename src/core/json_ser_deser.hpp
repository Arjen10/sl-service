//
// Created by Arjen on 2025/10/30.
//

#ifndef SL_SERVICE_JSON_SER_DESER_HPP
#define SL_SERVICE_JSON_SER_DESER_HPP

#include <boost/date_time/posix_time/posix_time.hpp>
#include <nlohmann/json.hpp>

#define JSON_FIELD_PTR(j, obj, field) j[#field] = obj->field

#define JSON_FIELD_REF(j, obj, field) j[#field] = obj.field

namespace boost::posix_time {
    void to_json(nlohmann::json& j, const boost::posix_time::ptime& ptime);
}

#endif // SL_SERVICE_JSON_SER_DESER_HPP
