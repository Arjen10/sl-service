//
// Created by Arjen on 2025/10/22.
//

#ifndef SL_SERVICE_PROTOCOL_FACTORY_HPP
#define SL_SERVICE_PROTOCOL_FACTORY_HPP

#include <unordered_map>
#include "../basic_decoder.hpp"
#include "sl651_2014.hpp"

class protocol_factory {

  public:
    static std::shared_ptr<sl_basic_decoder> create(int16_t key);

  private:
    using factory_map = std::unordered_map<int16_t, std::function<std::shared_ptr<sl_basic_decoder>()>>;

    static const factory_map _map;
};

#endif // SL_SERVICE_PROTOCOL_FACTORY_HPP
