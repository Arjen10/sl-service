////
//// Created by Arjen on 2025/1/21.
////
//
// #ifndef PARSE_STRATEGY_HPP
// #define PARSE_STRATEGY_HPP
//
// #include "slt427_2021.hpp"
// #include "../../element_parse_strategy.hpp"
//
// namespace slt427_2021
//{
// class header;
// class content;
// class end;
//}
//
// namespace slt427_2021_parse_strategy
//{
//
// class mp_q_r: public element_parse_strategy<slt427_2021::header, slt427_2021::content, slt427_2021::end>
//{
//
// public:
//    mp_q_r() = default;
//
//    mp_q_r(const mp_q_r &) = delete;
//
//    mp_q_r &operator=(const mp_q_r &) = delete;
//
// public:
//
//    virtual void operator()(slt427_2021::header *h, slt427_2021::content *c,
//                            slt427_2021::end *e, byte_buf_reader_helper &reader_helper) override;
//};
//
//}
//
//
// #endif //PARSE_STRATEGY_HPP
