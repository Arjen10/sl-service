//
// Created by Arjen on 2025/1/8.
//

#ifndef BYTE_HELPER_HPP
#define BYTE_HELPER_HPP

// 提取字节的高四位和低四位
#define split_byte(b, high_four_bits, low_four_bits)  \
    high_four_bits = ((b) >> 4) & 0xF;                \
    low_four_bits = (b) & 0xF;



#endif //BYTE_HELPER_HPP
