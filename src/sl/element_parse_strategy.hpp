//
// Created by Arjen on 2025/3/9.
//

#ifndef ELEMENT_PARSE_STRATEGY_HPP
#define ELEMENT_PARSE_STRATEGY_HPP

#include <unordered_map>
#include <memory>
#include <functional>

#include "byte_buf_reader.hpp"

class element_parse_strategy_base;

/**
 * 要素解析策略器
 * @tparam header 头部对象
 * @tparam content 正文对象
 * @tparam end 结束对象
 */
class element_parse_strategy_base {

  public:
    /**
     * 开始执行解析
     * @param data 字节指针
     * @param p_size 指针大小
     */
    virtual void parse(void* h, void* c, void* e, byte_buf_reader& reader_helper) = 0;
};

template <typename Header, typename Content, typename End>
class element_parse_strategy : public element_parse_strategy_base {

  public:
    /**
     * 模版类执行类型转换，子类只需要关注仿函数即可
     * @param h
     * @param c
     * @param e
     * @param reader_helper
     */
    void parse(void* h, void* c, void* e, byte_buf_reader& reader_helper) override {
        this->operator()(reinterpret_cast<Header*>(h), reinterpret_cast<Content*>(c), reinterpret_cast<End*>(e),
                         reader_helper);
    }

  private:
    virtual void operator()(Header* h, Content* c, End* e, byte_buf_reader& reader_helper) = 0;
};

class strategy_factory {
  public:
    using create_func = std::function<std::shared_ptr<element_parse_strategy_base>()>;

    static strategy_factory& instance();

    void register_strategy(char c, create_func&& creator);

    std::shared_ptr<element_parse_strategy_base> get_strategy(char c);

  private:
    std::unordered_map<char, create_func> registry;
};

#endif // ELEMENT_PARSE_STRATEGY_HPP
