//
// Created by Arjen on 2025/11/7.
//

#include "except.hpp"

namespace parse {

    error::error(std::string msg) : full_msg(std::move(msg)) {
    }

    const char* error::what() const throw() {
        return full_msg.c_str();
    }

} // namespace parse
