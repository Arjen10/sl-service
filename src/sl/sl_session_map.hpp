//
// Created by Arjen on 2025/10/30.
//

#ifndef SL_SERVICE_SL_SESSION_MAP_HPP
#define SL_SERVICE_SL_SESSION_MAP_HPP

#include <unordered_map>
#include <memory>
#include <mutex>
#include <optional>

#include "sl_session.hpp"

class sl_session_map {

  private:
    /**
     * 锁
     */
    std::mutex mutex_;

    /**
     * session map
     */
    std::unordered_map<std::string, std::shared_ptr<sl_session>> session_map_;

    sl_session_map() = default;

  public:
    // 获取唯一实例
    static sl_session_map& instance();

    /**
     * 通过 uuid 查找一个水利会话指针
     * @param uuid uuid
     * @return 水利会话指针
     */
    std::optional<std::shared_ptr<sl_session>> find_by_uuid(const std::string& uuid);

    /**
     * 线程安全的插入一个会话指针
     * @param session_ptr
     */
    void thread_safe_insert(const std::shared_ptr<sl_session>& session_ptr);

    // 禁止拷贝和赋值操作
    sl_session_map(const sl_session_map&) = delete;

    sl_session_map& operator=(const sl_session_map&) = delete;
};

#endif // SL_SERVICE_SL_SESSION_MAP_HPP
