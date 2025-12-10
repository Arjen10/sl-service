//
// Created by Arjen on 2025/11/5.
//

#ifndef SL_SERVICE_CALLBACK_METHOD_HPP
#define SL_SERVICE_CALLBACK_METHOD_HPP

enum method : uint16_t {

    /**
     * http回调
     */
    http_callback = 1,

    /**
     * MQTT
     */
    mqtt_callback = 2,

    /**
     * 未知
     */
    unknown = 999,

};

#endif //SL_SERVICE_CALLBACK_METHOD_HPP
