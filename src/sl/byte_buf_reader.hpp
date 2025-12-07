//
// Created by Arjen on 2025/1/21.
//

#ifndef BYTE_BUF_READER_HELPER_HPP
#define BYTE_BUF_READER_HELPER_HPP

#include <cstddef>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <boost/asio/buffer.hpp>
#include <boost/date_time.hpp>

/**
 * 字节缓冲读取小助手
 */
class byte_buf_reader
{

private:

    /**
     * boost const_buffer
     */
    const boost::asio::const_buffer &buffer_;

    /**
     * const_buffer 对应的首地址指针
     */
    char const *p_;

    /**
     * 当前读取索引
     */
    std::size_t read_index_;

public:

    explicit byte_buf_reader(const boost::asio::const_buffer &buffer);

    ~byte_buf_reader() = default;

    /**
     * 读取一个字节
     * @param byte 字节引用
     */
    void read_byte(int8_t &byte);

    /**
     * 读取一个字节
     * @param byte 字节引用
     */
    int8_t read_byte();

    /**
     * 获取指定下标的字节
     * @param byte 字节引用
     * @param index 从开始
     */
    void get_byte(int8_t &byte, std::size_t index);

    /**
     * 获取字节，不移动读指针
     * @param bytes
     * @param length
     */
    void get_bytes(std::vector<int8_t> &bytes, std::size_t start, std::size_t length);

    /**
     * 读取一个无符号位整数
     * @param byte 字节引用
     */
    void read_byte(u_int8_t &byte);

    /**
     * 读取指定长度的字节
     * @param bytes_ 字节序列容器
     * @param length 读取的长度
     */
    void read_bytes(std::vector<int8_t> &bytes, std::size_t length);

    /**
     * 读取一个短整型
     * @param s_ 短整型引用
     */
    void read_short(int16_t &s);

    /**
     * 获取short，不移动读指针
     * @param s
     * @param index 从多少位置开始
     */
    void get_short(int16_t &s, std::size_t index);

    /**
     * 读取hex字符串
     * @param str hex字符串，将结果追加到字符串后面
     * @param length 读取长度
     */
    void read_hex_str(std::string &str, std::size_t length);

    /**
     * 从当前读指针开始读取指定长度
     * @param str hex字符串
     * @param length 读取指定长度
     */
    void get_hex_str(std::string &str, std::size_t length);

    /**
     * 从指定索引开始读取指定长度
     * @param str hex字符串
     * @param start_index 指定索引
     * @param length 读取指定长度
     */
    void get_hex_str(std::string &str, std::size_t start_index, std::size_t length);

    /**
     * 返回剩余可读字节数
     * @return 剩余可读字节数
     */
    std::size_t readable_bytes() const;

    /**
     * 返回读取索引
     */
    std::size_t read_index() const;

    /**
     * 跳过指定数量的字节
     * @param length  需要跳过的数量
     */
    void skip(std::size_t length);

    /**
     * 将bcd字节读取为int
     */
    int read_bcd_byte_to_int(std::size_t length);

    /**
     * 读取时间
     * @param time_length 取几个字节
     * @param tm 时间引用
     */
    void read_tm(boost::posix_time::ptime &tm, std::size_t time_length);

    /**
     * coding by chat gpt
     */
    std::string debug();

};

#endif //BYTE_BUF_READER_HELPER_HPP
