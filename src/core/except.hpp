//
// Created by Arjen on 2025/11/7.
//

#ifndef SL_SERVICE_EXCEPT_HPP
#define SL_SERVICE_EXCEPT_HPP

#include <stdexcept>
#include <string>
#include <utility>

namespace parse {

    /**
     * 解析异常类
     */
    class error : public std::exception {
      private:
        std::string full_msg;

      public:
        explicit error(std::string msg = "parse error");

        virtual ~error() throw() {
        }

        virtual const char* what() const throw();
    };

} // namespace parse

#endif // SL_SERVICE_EXCEPT_HPP
