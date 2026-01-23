//
// Created by Arjen on 2025/10/30.
//

#include "sl_session_map.hpp"

sl_session_map& sl_session_map::instance() {
    static sl_session_map instance;
    return instance;
}

std::optional<std::shared_ptr<sl_session>> sl_session_map::find_by_uuid(const std::string& uuid) {
    auto it = this->session_map_.find(uuid);
    if (it != this->session_map_.end()) {
        return it->second;
    }
    return std::nullopt;
}

void sl_session_map::thread_safe_insert(const std::shared_ptr<sl_session>& session_ptr) {
    std::lock_guard<std::mutex> guard(this->mutex_);
    this->session_map_[session_ptr->uuid()] = session_ptr;
}
