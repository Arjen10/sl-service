//
// Created by Arjen on 2025/3/2.
//

#ifndef PROTOCOL_DEFINE_HPP
#define PROTOCOL_DEFINE_HPP

#include <string>
#include <memory>
#include <map>
#include "boost/asio/buffer.hpp"
#include "boost/beast.hpp"

namespace beast = boost::beast; // from <boost/beast.hpp>

class protocol_define {
  public:
    /**
     * 报文头的最大字节数量
     */
    virtual std::size_t header_max_len() = 0;

    /**
     * 通过字节缓冲区获取正文长度
     * @param buffer 字节缓冲通常是头部字节
     * @return 正文长度
     */
    virtual std::size_t content_length(const boost::asio::const_buffer& buffer) = 0;

    /**
     * 通过const_buffer来确定版本
     * @return 是否是当前版本
     */
    virtual bool version_by_buf(const boost::asio::const_buffer& buffer) = 0;

    /**
     * 结尾校验符、结束符长度
     */
    virtual std::size_t end_length() = 0;

    /**
     * 协议名称
     */
    virtual std::string protocol_name() = 0;
};

#endif // PROTOCOL_DEFINE_HPP
