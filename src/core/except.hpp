//
// Created by Arjen on 2025/11/7.
//

#ifndef SL_SERVICE_EXCEPT_HPP
#define SL_SERVICE_EXCEPT_HPP

#include <stdexcept>
#include <string>

namespace parse {

    /**
    * 解析异常类
    */
    class error : std::exception {
    private:
        std::string full_msg;
    };

}

#endif //SL_SERVICE_EXCEPT_HPP
