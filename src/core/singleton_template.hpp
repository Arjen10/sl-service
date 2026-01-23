//
// Created by Arjen on 2025/11/21.
//

#ifndef SL_SERVICE_SINGLETON_TEMPLATE_HPP
#define SL_SERVICE_SINGLETON_TEMPLATE_HPP

#include <utility>

template <typename type> class singleton_template {

  public:
    // 禁止拷贝和移动
    singleton_template(const singleton_template&) = delete;
    singleton_template& operator=(const singleton_template&) = delete;
    singleton_template(singleton_template&&) = delete;
    singleton_template& operator=(singleton_template&&) = delete;

    static type& instance() {
        static type singleton;
        return singleton;
    }

  protected:
    // 允许派生类调用
    singleton_template() = default;
    ~singleton_template() = default;
};

#endif // SL_SERVICE_SINGLETON_TEMPLATE_HPP
