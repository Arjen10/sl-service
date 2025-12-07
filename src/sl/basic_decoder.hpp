//
// Created by Arjen on 2024/12/30.
//

#ifndef BASIC_DECODER_HPP
#define BASIC_DECODER_HPP

#define CONTENT_TYPE "Content-Type"
#define APP_JSON "application/json"

#include <boost/beast.hpp>
#include <boost/log/trivial.hpp>
#include <boost/asio/streambuf.hpp>
#include <string>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

#include "../core/log.hpp"
#include "../core/json_ser_deser.hpp"
#include "../core/server_init.hpp"
#include "sl_full_buf.hpp"
#include "sl_parser_base.hpp"
#include "protocol_define.hpp"
#include "byte_buf_reader.hpp"

namespace beast = boost::beast;                 // from <boost/beast.hpp>

using crc_value_t = std::variant<int16_t, int8_t>;

/**
 * 封装报文数据对应的注释和实际取值
 */
class raw_hex_value {

public:

    /**
     * 原始报文
     */
    std::string _raw_hex;

    /**
    * 原始报文字节长度
    */
    uint16_t _length;

    /**
     * 原始报文的含义
     */
    std::string _note;

    /**
     * 原始报文取值
     */
    std::string _v;

public:
    friend void to_json(nlohmann::json &j, const raw_hex_value &value);

    explicit raw_hex_value(byte_buf_reader &reader, uint16_t length,
                           std::string note, std::string v);

    explicit raw_hex_value(byte_buf_reader &reader, uint16_t length,
                           std::string note, const nlohmann::json &v);

    explicit raw_hex_value(byte_buf_reader &reader, uint16_t length,
                           std::string note);

    /**
     * 更新数据
     * @param raw_hex 在原始基础上追加
     * @param v 直接覆盖
     */
    void update(byte_buf_reader &reader, uint16_t length, const std::string &v);

};

class base_decoded_result {

public:

    /**
     * 报文和值说明，用 list 就好，这里就是插入不需要查询
     */
    std::list<raw_hex_value> _raw_list;

};

/**
 * 水利协议基类，定义了很多函数，各自的协议根据自己的标准实现
 */
class sl_basic_decoder : public protocol_define {

public:

    virtual ~sl_basic_decoder() = default;

    /**
     * 解析模板虚函数
     * @param sl_full_buf 切包完成的 buffer
     */
    virtual std::optional<std::shared_ptr<asio::streambuf>> parse_template(const sl_full_buf &sl_full_buf) = 0;

    /**
     * CRC校验
     * @return 接收到的报文是否符合crc要求
     */
    virtual bool crc_check(const boost::asio::const_buffer &header,
                           const boost::asio::const_buffer &content,
                           const boost::asio::const_buffer &end) = 0;

    /**
     * CRC 计算
     */
    virtual crc_value_t crc_calculate(const boost::asio::const_buffer &header,
                                      const boost::asio::const_buffer &content,
                                      const boost::asio::const_buffer &end) = 0;
};

template<typename base, typename h, typename c, typename e>
class sl_decoder_crtp : public sl_basic_decoder {

    std::optional<std::shared_ptr<asio::streambuf>> parse_template(const sl_full_buf &sl_full_buf) override {
        base *base_ptr = static_cast<base *>(this);
        const std::shared_ptr<h> h_ptr = base_ptr->parse_header(sl_full_buf.header());
        const std::shared_ptr<c> c_ptr = base_ptr->parse_content(h_ptr, sl_full_buf.content());
        const std::shared_ptr<e> e_ptr = base_ptr->parse_end(h_ptr, c_ptr, sl_full_buf.end());
        base_ptr->do_something(h_ptr, c_ptr, e_ptr);
        return base_ptr->resp_byte_buffer(h_ptr, c_ptr, e_ptr);
    }

};

namespace callback {

    namespace chrono = std::chrono;

    enum method : uint16_t {

        /**
         * http回调
         */
        http = 1,

    };

    /**
     * 回调函数
     */
    using func = std::function<void(const nlohmann::json &h_j,
                                    const nlohmann::json &c_j,
                                    const nlohmann::json &e_j)>;

    /**
     * 存放标识符的解析函数
     */
    extern const std::unordered_map<method, func> _map;

}


#endif //BASIC_DECODER_HPP
