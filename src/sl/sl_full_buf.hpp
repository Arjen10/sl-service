//
// Created by Arjen on 2025/3/2.
//

#ifndef SL_FULL_BUF_HPP
#define SL_FULL_BUF_HPP

#include <boost/asio/streambuf.hpp>
#include <ostream>

namespace asio = boost::asio;

/**
 * 完整的一个包，分为头部、正文、结束部分
 */
class sl_full_buf
{

public:

    sl_full_buf();

    ~sl_full_buf() = default;

    sl_full_buf(sl_full_buf&&) noexcept;

    asio::const_buffer header() const;

    asio::const_buffer content() const;

    asio::const_buffer end() const;

    /**
     * 写入头部
     */
    void write_header(const int8_t* data, size_t size);

    /**
     * 写入正文部分
     */
    void write_content(const int8_t* data, size_t size);

    /**
     * 写入结束部分
     */
    void write_end(const int8_t* data, size_t size);

    /**
     * 重置
     */
    void reset();

private:

    std::unique_ptr<asio::streambuf> _header;

    std::unique_ptr<asio::streambuf> _content;

    std::unique_ptr<asio::streambuf> _end;

    /**
     * 写入字节
     */
    void write(const int8_t* data, size_t size, std::unique_ptr<asio::streambuf> &buf);

    /**
     * 写入字节
     */
    void write(int8_t byte, std::unique_ptr<asio::streambuf> &buf);

};


#endif //SL_FULL_BUF_HPP
