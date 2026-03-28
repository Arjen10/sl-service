//
// Created by Arjen on 2024/12/30.
//

#ifndef BASIC_DECODER_HPP
#define BASIC_DECODER_HPP

#include <boost/beast.hpp>
#include <boost/asio/streambuf.hpp>
#include <string>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>
#include <spdlog/spdlog.h>

#include "../core/json_ser_deser.hpp"
#include "../core/server_init.hpp"
#include "sl_full_buf.hpp"
#include "sl_parser_base.hpp"
#include "protocol_define.hpp"
#include "byte_buf_reader.hpp"
#include "../core/callback_method.hpp"

namespace beast = boost::beast;

using crc_value_t = std::variant<int16_t, int8_t>;

namespace callback {

    namespace chrono = std::chrono;

    /**
     * 回调函数
     */
    using func = std::function<void(const std::string& stcd, const nlohmann::json& h_j, const nlohmann::json& c_j,
                                    const nlohmann::json& e_j)>;

    /**
     * 存放标识符的解析函数，不要抛出异常打印日志即可
     */
    extern const std::unordered_map<method, func> _map;

} // namespace callback

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

    /**
     * 是否为拓展标识符
     */
    bool _extended;

  public:
    friend void to_json(nlohmann::json& j, const raw_hex_value& value);

    explicit raw_hex_value(byte_buf_reader& reader, uint16_t length, std::string note, std::string v,
                           bool extended = false);

    explicit raw_hex_value(byte_buf_reader& reader, uint16_t length, std::string note, const nlohmann::json& v,
                           bool extended = false);

    explicit raw_hex_value(byte_buf_reader& reader, uint16_t length, std::string note, bool extended = false);

    /**
     * 更新数据
     * @param raw_hex 在原始基础上追加
     * @param v 直接覆盖
     */
    void update(byte_buf_reader& reader, uint16_t length, const std::string& v);
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
    virtual std::optional<std::shared_ptr<asio::streambuf>> parse_template(const sl_full_buf& sl_full_buf) = 0;

    /**
     * CRC校验
     * @return 接收到的报文是否符合crc要求
     */
    virtual bool crc_check(const boost::asio::const_buffer& header, const boost::asio::const_buffer& content,
                           const boost::asio::const_buffer& end) = 0;

    /**
     * CRC 计算
     */
    virtual crc_value_t crc_calculate(const boost::asio::const_buffer& header, const boost::asio::const_buffer& content,
                                      const boost::asio::const_buffer& end) = 0;
};

template <typename base, typename header, typename content, typename end>
class sl_decoder_crtp : public sl_basic_decoder {

    /**
     * 解析模版，将解析报文头、报文正文、报文结束部分拆分
     * @param sl_full_buf 水利完整 buffer
     * @return
     */
    std::optional<std::shared_ptr<asio::streambuf>> parse_template(const sl_full_buf& sl_full_buf) override {
        base* base_ptr = static_cast<base*>(this);
        const std::shared_ptr<header> h_ptr = base_ptr->parse_header(sl_full_buf.header());
        const std::shared_ptr<content> c_ptr = base_ptr->parse_content(h_ptr, sl_full_buf.content());
        const std::shared_ptr<end> e_ptr = base_ptr->parse_end(h_ptr, c_ptr, sl_full_buf.end());
        base_ptr->do_something(h_ptr, c_ptr, e_ptr);
        return base_ptr->resp_byte_buffer(h_ptr, c_ptr, e_ptr);
    }

  protected:
    /**
     * 回调业务端，具体回调方式由配置文件决定
     * @param stcd 测站编码
     * @param h 报头
     * @param c 报体
     * @param e 报尾
     */
    void call_back(const std::string& stcd, const std::shared_ptr<header>& h, const std::shared_ptr<content>& c,
                   const std::shared_ptr<end>& e) {
        auto& back_cfg = conf::sl::instance().get_callback();
        // 回调未开启，直接跳过
        if (!back_cfg.enable()) {
            return;
        }
        auto& m_opt = back_cfg.get_callback_method();
        auto it = callback::_map.find(m_opt.value());
        if (it == callback::_map.end()) {
            spdlog::error("未找到对应的回调函数，请检查配置文件 callback.method 项目");
            return;
        }
        nlohmann::json h_json = h;
        nlohmann::json c_json = c;
        nlohmann::json e_json = e;
        // 开始回调
        it->second(stcd, h_json, c_json, e_json);
    }
};

#endif // BASIC_DECODER_HPP
