//
// Created by Arjen on 2025/3/9.
//

#include "element_parse_strategy.hpp"

strategy_factory &strategy_factory::instance() {
    static strategy_factory factory;
    return factory;
}

void strategy_factory::register_strategy(char c, strategy_factory::create_func &&creator) {
    registry[c] = creator;
}

std::shared_ptr<element_parse_strategy_base> strategy_factory::get_strategy(char c) {
    auto it = registry.find(c);
    if (it != registry.end()) {
        return it->second();
    }
    return nullptr;
}