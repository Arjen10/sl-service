//
// Created by Arjen on 2025/12/7.
//

#ifndef SL_SERVICE_MQTT5_CLIENT_HPP
#define SL_SERVICE_MQTT5_CLIENT_HPP

#include <boost/mqtt5/logger.hpp>
#include <boost/mqtt5/mqtt_client.hpp>
#include <boost/mqtt5/types.hpp>

#include <memory>
#include <mutex>

#include "singleton_template.hpp"
#include "server_init.hpp"

using mqtt_client_type = boost::mqtt5::mqtt_client<boost::asio::ip::tcp::socket, std::monostate, boost::mqtt5::logger>;

class mqtt5_client : public singleton_template<mqtt5_client> {
  private:
    // 重连定时器
    std::shared_ptr<boost::asio::steady_timer> _reconnect_timer;
    // 是否是连接状态
    std::atomic<bool> _is_connected{false};
    std::shared_ptr<mqtt_client_type> _client;
    friend class singleton_template<mqtt5_client>;

  public:
    /**
     * 初始化
     */
    void init(boost::asio::io_context& ioc);

    /**
     * 异步发布消息
     * stcd 和 message 参数在 boost mqtt5 客户端中会发生值拷贝，这里传常量引用即可
     * @param stcd 测站编码
     * @param message 消息
     */
    void publish_async(const std::string& stcd, const std::string& message);
};

#endif // SL_SERVICE_MQTT5_CLIENT_HPP
