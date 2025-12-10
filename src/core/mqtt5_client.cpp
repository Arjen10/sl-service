//
// Created by Arjen on 2025/12/7.
//

#include <boost/mqtt5/types.hpp>

#include "mqtt5_client.hpp"
#include "log.hpp"

void mqtt5_client::init(boost::asio::io_context &ioc) {
    _client = std::make_shared<mqtt_client_type>(
            ioc, std::monostate{}, boost::mqtt5::logger(boost::mqtt5::log_level::info)
    );
    auto &cfg = conf::sl::instance().get_callback().get_mqtt_conf();
    // 设置 broker、clientId、持久会话 等
    _client->brokers(cfg->get_broker_host(), cfg->get_broker_port())
            .credentials(cfg->get_client_id(), cfg->get_username(), cfg->get_password())
            .connect_property(boost::mqtt5::prop::session_expiry_interval, 60);
    // 开始异步运行（保持连接）
    _client->async_run(
            [this](boost::mqtt5::error_code ec) {
                if (!ec) {
                    // 正常退出？一般不会发生，除非主动 stop
                    LOG_INFO << "MQTT client stopped gracefully.";
                    return;
                }
                LOG_ERROR << "MQTT connection lost: " << ec.message();
            }
    );
}

void mqtt5_client::publish_async(const std::string &stcd, const std::string &message) {
    auto &cfg = conf::sl::instance().get_callback().get_mqtt_conf();
    std::string topic = cfg->get_topic_prefix() + "/" + stcd;
    auto qos = cfg->get_qos();
    auto publish_callback = [this, topic, message](auto ec, auto&&...) {
        if (!ec) {
            LOG_DEBUG << "[MQTT publish OK] topic= " << topic;
            return;
        }
        LOG_ERROR << "[MQTT 发布失败] topic= " << topic
                  << " message= " << message
                  << " ec=" << ec.message();
    };

    // 根据运行时 QoS 值分发到对应的模板实例
    switch (qos) {
        case boost::mqtt5::qos_e::at_most_once:
            this->_client->async_publish<boost::mqtt5::qos_e::at_most_once>(
                    topic, message,boost::mqtt5::retain_e::yes,
                    boost::mqtt5::publish_props{},
                    // 必须 move 进去 async_publish 是异步的，必须保证对象的生命周期
                    std::move(publish_callback)
            );
            break;
        case boost::mqtt5::qos_e::at_least_once:
            this->_client->async_publish<boost::mqtt5::qos_e::at_least_once>(
                    topic, message,
                    boost::mqtt5::retain_e::yes,
                    boost::mqtt5::publish_props{},
                    std::move(publish_callback)
            );
            break;
        case boost::mqtt5::qos_e::exactly_once:
            this->_client->async_publish<boost::mqtt5::qos_e::exactly_once>(
                    topic, message,
                    boost::mqtt5::retain_e::yes,
                    boost::mqtt5::publish_props{},
                    std::move(publish_callback)
            );
            break;
        default:
            // 按道理来说不会走到这里，打个日志提醒下
            LOG_WARN << "错误的 qos 配置 " << static_cast<std::int32_t>(qos) << " 消息跳过发送";
            break;
    }

}