////
//// Created by Arjen on 2025/1/21.
////
//
//#include "parse_strategy.hpp"
//
//// ----------------------------------------------------afn_parse--------------------------------------------------
//
//void parse_qr(byte_buf_reader &reader_helper, double &value, bool is_q = true)
//{
//    double temp = 0;
//    int high_four_bits;
//    int low_four_bits;
//    char c;
//    // 第一个字节
//    reader_helper.read_byte(c);
//    split_byte(c, high_four_bits, low_four_bits);
//    // (百分位 或者 十位) + (千分位 或者 个位)
//    temp += high_four_bits * 0xA + low_four_bits;
//    // 第二个字节
//    reader_helper.read_byte(c);
//    split_byte(c, high_four_bits, low_four_bits);
//    // (个位 或者 千位) + (十分位 或者 百位)
//    temp += high_four_bits * 0x3E8 + low_four_bits * 0x64;
//    // 第三个字节
//    reader_helper.read_byte(c);
//    split_byte(c, high_four_bits, low_four_bits);
//    // (百位 或者 十万位) + (十位 或者 万位)
//    temp += high_four_bits * 0x186A0 + low_four_bits * 0x2710;
//    // 第四个个字节
//    reader_helper.read_byte(c);
//    split_byte(c, high_four_bits, low_four_bits);
//    //  (万位 或者 千万位) + (千位 或者 百万位)
//    temp += high_four_bits * 0x989680 + low_four_bits * 0xF4240;
//    // 第五个字节
//    reader_helper.read_byte(c);
//    auto b5 = c;
//    //  (十万位 或者 亿位)
//    temp += (b5 & 0x0f) * 0x5F5E100;
//    // 处理水量
//    if (!is_q) {
//        //水量，十亿位
//        int billion = (b5 & 0x7) * 0x3B9ACA00;
//        value = temp + billion;
//        return;
//    }
//    // 处理流量
//    // 无符号右移 4 位，获取高四位
//    unsigned char upperFourDigits = (b5 >> 4);
//    // 获取低两位作为单位
//    int unit = upperFourDigits & 0x3;
//    // 符号位，根据高两位来判断符号
//    int symbol = (upperFourDigits >> 2) == 0 ? 1 : -1;
//    value = unit == 0 ? temp * symbol * 0.001
//                      : (temp * 0.001 / 3600) * symbol;
//}
//
//void slt427_2021_parse_strategy::mp_q_r::operator()(slt427_2021::header *h,
//                                                    slt427_2021::content *c,
//                                                    slt427_2021::end *e,
//                                                    byte_buf_reader &reader_helper)
//{
//    std::size_t user_data_length = reader_helper.readable_bytes() - slt427_2021::TIME_LENGTH - slt427_2021::RTU_STATE;
//    // 除以10得到N，查看文档第23和30页
//    auto n = user_data_length / 10;
//    double total_q = 0;
//    double total_r = 0;
//    for (int i = 0; i < n; ++i) {
//        // 第一组五字节是流量
//        double temp;
//        parse_qr(reader_helper, temp);
//        total_q += temp;
//        parse_qr(reader_helper, temp, false);
//        total_r += temp;
//    }
//    // 我们这里取平均
//    auto n_d = static_cast<double>(n);
//    auto q = total_q / n_d;
//    auto r = total_r / n_d;
//    c->wr_mp_q_r = std::make_shared<wr_mp_q_r>();
//    c->wr_mp_q_r->mp_q = q;
//    c->wr_mp_q_r->acc_w = r;
//    c->wr_mp_q_r->tm = c->time;
//}
