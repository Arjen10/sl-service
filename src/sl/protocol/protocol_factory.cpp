//
// Created by Arjen on 2025/10/22.
//

#include "protocol_factory.hpp"

const protocol_factory::factory_map protocol_factory::_map = {

        {SL651_2014_VER_HEX, []() { return std::make_shared<sl651_2014::codec::decoder>(); }},

};

std::shared_ptr<sl_basic_decoder> protocol_factory::create(int16_t key) {
    auto it = _map.find(key);
    if (it != _map.end()) {
        return it->second();
    }
    return nullptr;
}
