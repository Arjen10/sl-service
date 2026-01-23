//
// Created by Arjen on 2024/12/21.
//

#ifndef SL_PARSER_BASE_HPP
#define SL_PARSER_BASE_HPP

enum class decode_state {

    /**
     * 头部读取
     */
    read_header,

    /**
     * 读取固定长度的正文部分
     */
    read_fixed_length_content,

    /**
     * 读取结束部分
     */
    read_end_section,

    /**
     * 读取结束
     */
    done,

};

#endif // SL_PARSER_BASE_HPP
