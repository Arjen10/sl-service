//
// Created by Arjen on 2025/1/21.
//

#ifndef BYTE_BUF_WRITER_HPP
#define BYTE_BUF_WRITER_HPP

#include <cstdint>
#include <cstddef>
#include <sstream>
#include <iomanip>
#include <vector>
#include <stdexcept>
#include <string>

#include <boost/asio/streambuf.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

/**
 * 字节缓冲写入小助手
 */
class byte_buf_writer {

  private:
    /**
     * boost streambuf 引用
     */
    boost::asio::streambuf& _buf;

    /**
     * 输出流
     */
    std::ostream _os;

  public:
    /**
     * 构造函数
     * @param buf boost::asio::streambuf 引用
     */
    explicit byte_buf_writer(boost::asio::streambuf& buf);

    ~byte_buf_writer() = default;

    /**
     * 写入一个字节
     * @param byte 字节值
     */
    void write_byte(std::int8_t byte);

    /**
     * 写入一个无符号字节
     * @param byte 字节值
     */
    void write_byte(std::uint8_t byte);

    /**
     * 写入字节序列
     * @param bytes 字节序列容器
     */
    void write_bytes(const std::vector<int8_t>& bytes);

    /**
     * 写入指定长度的字节数据
     * @param data 数据指针
     * @param length 数据长度
     */
    void write_bytes(const char* data, std::size_t length);

    /**
     * 写入一个短整型(2字节, 大端序)
     * @param s 短整型值
     */
    void write_short(std::int16_t s);

    /**
     * 写入hex字符串(会将hex字符串解码为字节写入)
     * @param hex hex字符串, 如 "48656C6C6F"
     */
    void write_hex_str(const std::string& hex);

    /**
     * 将整数转换为BCD编码写入
     * @param value 整数值
     */
    void write_bcd_byte_from_int64(std::int64_t value);

    /**
     * 写入时间(BCD编码格式)
     * @param tm 时间
     * @param time_length 时间长度(5或6字节)
     */
    void write_tm(const boost::posix_time::ptime& tm, std::size_t time_length);

    /**
     * 返回已写入的字节数
     * @return 已写入字节数
     */
    std::size_t size() const;

    /**
     * 调试输出, 以十六进制和ASCII格式打印缓冲区内容
     * @return 格式化字符串
     */
    std::string debug();
};

#endif // BYTE_BUF_WRITER_HPP
