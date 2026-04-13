//
// Created by Arjen on 2025/9/12.
//

#include <spdlog/spdlog.h>

#include "sl651_2014.hpp"
#include "../byte_buf_writer.hpp"

namespace sl651_2014::json {

    void to_json(nlohmann::json& j, const model::req_type& type) {
        switch (type) {
        case model::req_type::MANUAL_DATA:
            j = "遥测站人工置数报";
            break;
        case model::req_type::KEEP_ALIVE:
            j = "链路维持报";
            break;
        case model::req_type::HYDROLOGY_PERIODIC:
            j = "均匀时段水文信息报";
            break;
        case model::req_type::REGULAR:
            j = "遥测站定时报";
            break;
        case model::req_type::ADDITIONAL:
            j = "遥测站加报报";
            break;
        case model::req_type::HOURLY:
            j = "遥测站小时报";
            break;
        default:
            j = "未知类型";
            break;
        }
    }

    void to_json(nlohmann::json& j, const model::station_type& type) {
        switch (type) {
        case model::station_type::rr:
            j = "水库水文站";
            break;
        case model::station_type::pp:
            j = "雨量站";
            break;
        case model::station_type::wq:
            j = "水质站";
            break;
        case model::station_type::zq_zz:
            j = "河道水文站或者河道水位站";
            break;
        default:
            j = "未知类型";
            break;
        }
    }

    void to_json(nlohmann::json& j, const model::data_type& type) {
        switch (type) {
        case model::data_type::tt:
            j = "观测时间标识符";
            break;
        case model::data_type::st:
            j = "测站编码引导符";
            break;
        case model::data_type::c:
            j = "瞬时水温";
            break;
        case model::data_type::drp:
            j = "1小时内5分钟时段雨量";
            break;
        case model::data_type::drz1:
            j = "1小时内5分钟间隔相对水位1";
            break;
        case model::data_type::vt:
            j = "电压";
            break;
        case model::data_type::ph:
            j = "pH";
            break;
        case model::data_type::dox:
            j = "溶解氧";
            break;
        case model::data_type::cond:
            j = "电导率";
            break;
        case model::data_type::turb:
            j = "浊度";
            break;
        case model::data_type::chla:
            j = "叶绿素";
            break;
        case model::data_type::codmn:
            j = "高锰酸盐浓度";
            break;
        case model::data_type::nh4n:
            j = "氨氮";
            break;
        case model::data_type::tp:
            j = "总磷";
            break;
        case model::data_type::tn:
            j = "总氮";
            break;
        default:
            j = "未知类型";
            break;
        }
    }

} // namespace sl651_2014::json

namespace sl651_2014 {

    error::error(const std::string& msg) {
        this->full_msg.append("sl651_2014解析异常 ----> ").append(msg);
    }

    const char* error::what() const noexcept {
        return full_msg.c_str();
    }
} // namespace sl651_2014

namespace sl651_2014::model {

    void to_json(nlohmann::json& j, const h_shared_ptr& h) {
        j = nlohmann::json::object();
        JSON_FIELD_PTR(j, h, _central_address);
        JSON_FIELD_PTR(j, h, _rtu_stcd);
        JSON_FIELD_PTR(j, h, _pwd);
        JSON_FIELD_PTR(j, h, _function);
        JSON_FIELD_PTR(j, h, _is_up);
        JSON_FIELD_PTR(j, h, _content_length);
        JSON_FIELD_PTR(j, h, _start_character);
        JSON_FIELD_PTR(j, h, _raw_list);
    }

    void to_json(nlohmann::json& j, const c_shared_ptr& c) {
        j = nlohmann::json::object();
        JSON_FIELD_PTR(j, c, _serial_num);
        JSON_FIELD_PTR(j, c, _report_tm);
        JSON_FIELD_PTR(j, c, _stcd);
        JSON_FIELD_PTR(j, c, _station_type);
        JSON_FIELD_PTR(j, c, _v);
        JSON_FIELD_PTR(j, c, _awqmd);
        JSON_FIELD_PTR(j, c, _raw_list);
        JSON_FIELD_PTR(j, c, _extended);
    }

    void to_json(nlohmann::json& j, const e_shared_ptr& e) {
        j = nlohmann::json::object();
        JSON_FIELD_PTR(j, e, _end_note);
        JSON_FIELD_PTR(j, e, _crc);
        JSON_FIELD_PTR(j, e, _raw_list);
    }

    void content::init(const c_shared_ptr& c) {
        switch (c->_station_type) {
        case station_type::wq: {
            this->_awqmd.emplace(c->_stcd);
            this->_obwt.emplace(c->_stcd);
            return;
        }
        case rr:
        case pp:
        case zq_zz:
            return;
        }
    }

} // namespace sl651_2014::model

namespace sl651_2014::codec {

    std::string decoder::protocol_name() {
        static const std::string protocol_name = "SL651-2014";
        return protocol_name;
    }

    model::h_shared_ptr decoder::parse_header(const asio::const_buffer& hb) {
        auto h_ptr = std::make_shared<model::header>();
        byte_buf_reader reader(hb);
        byte_buf_reader hex_reader(hb);
        // 读取版本字节
        reader.skip(VERSION_CONTROL_LEN);
        h_ptr->_raw_list.emplace_back(hex_reader, VERSION_CONTROL_LEN, VERSION_CONTROL_NOTE, this->protocol_name());
        // 中心站址 1字节
        reader.read_byte(h_ptr->_central_address);
        h_ptr->_raw_list.emplace_back(hex_reader, CENTRAL_ADDR_LEN, CENTRAL_ADDR_NOTE,
                                      std::to_string(h_ptr->_central_address));

        // 测站编码 5个字节
        // 先读测站的前一个 hex
        reader.read_hex_str(h_ptr->_rtu_stcd, 1);
        if (h_ptr->_rtu_stcd == "00") {
            // 如果第一个 hex 等于 00
            h_ptr->_rtu_stcd.clear();
        }
        // 读取后面剩下测站的 4 位，共计五位
        reader.read_hex_str(h_ptr->_rtu_stcd, 4);
        h_ptr->_raw_list.emplace_back(hex_reader, RTU_STCD_LEN, RTU_STCD_NOTE, h_ptr->_rtu_stcd);
        // 密码 2字节
        h_ptr->_raw_list.emplace_back(hex_reader, PWD_LEN, PWD_NOTE);
        reader.read_hex_str(h_ptr->_pwd, PWD_LEN);
        // 功能码 1个字节
        int8_t tc;
        reader.read_byte(tc);
        h_ptr->_function = static_cast<model::req_type>(tc);
        nlohmann::json j;
        // 转成字符串
        json::to_json(j, h_ptr->_function);
        h_ptr->_raw_list.emplace_back(hex_reader, FUNCTION_LEN, FUNCTION_NOTE, j.get<std::string>());

        // 请求标识符 2个字节，上行还是下行，一般都是上行
        int16_t s;
        reader.read_short(s);
        // 取高四位 上下行标识符 0000 上行 1000 下行
        h_ptr->_is_up = ((static_cast<uint16_t>(s) >> 12) & 0xF) == 0;
        if (h_ptr->_is_up) {
            SPDLOG_TRACE("这是一个上行数据");
        }
        h_ptr->_content_length = this->content_length(hb);
        std::stringstream ss;
        ss << "当前报文类型为:" << (h_ptr->_is_up ? "上" : "下")
           << "行报文, 报文正文长度为: " << h_ptr->_content_length;
        // 报文上行标识符及长度
        h_ptr->_raw_list.emplace_back(hex_reader, REQ_LEN, REQ_NOTE, ss.str());
        // 报文起始帧 1字节
        h_ptr->_raw_list.emplace_back(hex_reader, MES_START_LEN, MES_START_NOTE);
        reader.read_byte(h_ptr->_start_character);
        if (model::const_symbol::stx != h_ptr->_start_character) {
            std::string what = std::string("错误的帧起始符 ");
            what.append(reinterpret_cast<const char*>(&h_ptr->_start_character), 1);
            throw sl651_2014::error(what);
        }
        // 报头正确性检验
        if (reader.readable_bytes() > 0) {
            throw sl651_2014::error("错误的头部帧");
        }
        return h_ptr;
    }

    model::c_shared_ptr decoder::parse_content(const model::h_shared_ptr& h_ptr, const asio::const_buffer& cb) {
        auto c_ptr = std::make_shared<model::content>();
        byte_buf_reader reader(cb);
        byte_buf_reader hex_reader(cb);
        // 流水号 2字节
        reader.read_short(c_ptr->_serial_num);
        c_ptr->_raw_list.emplace_back(hex_reader, SERIAL_NUM_LEN, SERIAL_NUM_NOTE, std::to_string(c_ptr->_serial_num));
        SPDLOG_TRACE("流水号 raw {}, raw_hex {}", c_ptr->_serial_num, c_ptr->_raw_list.back()._raw_hex);
        // 发报时间 6字节 250721013101
        reader.read_tm(c_ptr->_report_tm, REPORT_TM_LEN);
        nlohmann::json temp_j;
        to_json(temp_j, c_ptr->_report_tm);
        SPDLOG_TRACE("发报时间 {}", temp_j.get<std::string>());
        c_ptr->_raw_list.emplace_back(hex_reader, REPORT_TM_LEN, REPORT_TM_NOTE, temp_j);
        // 处理链路维持报，当可读的字节等于 0 的时候，就是设备发的心跳包
        if (reader.readable_bytes() == 0) {
            if (h_ptr->_function != model::req_type::KEEP_ALIVE) {
                throw sl651_2014::error("错误的帧和功能码！");
            }
            SPDLOG_TRACE("设备心跳包");
            return nullptr;
        }
        // 测站编码引导符 2字节
        int8_t stcd_lead_symbol = reader.read_byte();
        c_ptr->_raw_list.emplace_back(hex_reader, 1, STCD_LEAD_SYMBOL_NOTE);
        if (stcd_lead_symbol != static_cast<int8_t>(model::data_type::st)) {
            throw sl651_2014::error("获取测站编码错误！");
        }
        const parse::parse_function& st_func = parse::strategy.at(model::data_type::st);
        st_func(reader, hex_reader, c_ptr);

        // 开始处理测站类型 1个字节
        int8_t station_type_lead_symbol = reader.read_byte();
        c_ptr->_station_type = static_cast<model::station_type>(station_type_lead_symbol);
        temp_j.clear();
        json::to_json(temp_j, c_ptr->_station_type);
        c_ptr->_raw_list.emplace_back(hex_reader, STATION_TYPE_LEN, STATION_TYPE_NOTE, temp_j);
        std::optional<double> temp_opt;
        // 观测时间标识符
        int8_t tt_symbol = reader.read_byte();
        auto tt_key = static_cast<model::data_type>(tt_symbol);
        if (parse::strategy.count(tt_key) == 0) {
            throw sl651_2014::error("获取观测时间错误！");
        }
        parse::strategy.at(tt_key)(reader, hex_reader, c_ptr);
        // 有了观测时间执行初始化
        c_ptr->init(c_ptr);
        // 开始处理不定长部分
        while (true) {
            // 可读字节等于 0 直接返回
            if (reader.readable_bytes() == 0) {
                return c_ptr;
            }
            // 要素编码引导符，告诉我们这个是个什么要素
            int8_t element_lead_symbol = reader.read_byte();
            auto key = static_cast<model::data_type>(element_lead_symbol);
            // 如果是标准值，直接用内置的解析器封装成对象即可
            const auto strategy_it = parse::strategy.find(key);
            if (strategy_it != parse::strategy.end()) {
                temp_j.clear();
                json::to_json(temp_j, key);
                c_ptr->_raw_list.emplace_back(hex_reader, 1, temp_j.get<std::string>() + "标识符");
                const parse::parse_function& func = strategy_it->second;
                func(reader, hex_reader, c_ptr);
                continue;
            }
            // 如果不是标准值，解析后返回给业务端
            temp_opt.reset();
            auto data_len = parse::parse_data(reader, temp_opt);
            try {
                std::ostringstream oss;
                oss << std::setw(2) << std::setfill('0') << std::hex << (int32_t)element_lead_symbol;
                auto& expand_map = c_ptr->_extended;
                // 这里存入 hex 编码
                expand_map.emplace(oss.str(), temp_opt);
                std::string expand_note = "拓展标识符 " + oss.str();
                c_ptr->_raw_list.emplace_back(hex_reader, 1, expand_note, true);
                auto v_str = temp_opt ? std::to_string(temp_opt.value()) : "null";
                c_ptr->_raw_list.emplace_back(hex_reader, data_len, expand_note + "  取值", v_str, true);
                SPDLOG_WARN("不支持的标识符 {}, 值 {}", oss.str(), v_str);
            } catch (const std::exception& e) {
                SPDLOG_ERROR(e.what());
                throw;
            }
        }
    }

    model::e_shared_ptr decoder::parse_end(const model::h_shared_ptr& h_ptr, const model::c_shared_ptr& c_ptr,
                                           const asio::const_buffer& eb) {
        auto end_ptr = std::make_shared<model::end>();
        byte_buf_reader reader(eb);
        byte_buf_reader hex_reader(eb);
        //  先get出来再说
        end_ptr->_raw_list.emplace_back(hex_reader, END_LEN, END_NOTE);
        reader.read_byte(end_ptr->_end_note);
        if (end_ptr->_end_note == model::const_symbol::etb) {
            // todo 分包传输还没支持
            throw sl651_2014::error("etb 分包传输还没支持");
        }
        if (end_ptr->_end_note != model::const_symbol::etx) {
            throw sl651_2014::error("错误的结束符");
        }
        if (reader.readable_bytes() != 2) {
            // 应该永远走不到这里
            throw sl651_2014::error("错误的CRC");
        }
        // 读取原始报文存起来
        reader.read_short(end_ptr->_crc);
        end_ptr->_raw_list.emplace_back(hex_reader, END_CRC_LEN, END_CRC_NOTE, std::to_string(end_ptr->_crc));
        return end_ptr;
    }

    void decoder::do_something(const model::h_shared_ptr& h_ptr, const model::c_shared_ptr& c_ptr,
                               const model::e_shared_ptr& e_ptr) {
        this->call_back(h_ptr->_rtu_stcd, h_ptr, c_ptr, e_ptr);
    }

    /**
     * 7E 7E
     * 00 60 80 20 41
     * 01
     * 00 00
     * 32
     * 80 0E
     * 02
     * 46 5D
     * 26 04 11 14 27 11
     * F1
     * 00 60 80 20 41
     * 1B
     * 38 0D
     */
    std::optional<std::shared_ptr<asio::streambuf>> decoder::resp_byte_buffer(const model::h_shared_ptr& h_ptr,
                                                                              const model::c_shared_ptr& c_ptr,
                                                                              const model::e_shared_ptr& e_ptr) {
        if (h_ptr->_function == model::req_type::KEEP_ALIVE) {
            return std::nullopt;
        }
        auto sb = std::make_shared<asio::streambuf>();
        byte_buf_writer w(*sb);
        // 开始报头固定结构
        w.write_short(model::frame_symbol::start_and_end);
        std::string rtu_stcd = (h_ptr->_rtu_stcd.length() == 10 ? h_ptr->_rtu_stcd : ("00" + h_ptr->_rtu_stcd));
        w.write_hex_str(rtu_stcd);
        w.write_byte(h_ptr->_central_address);
        w.write_hex_str(h_ptr->_pwd);
        w.write_byte(static_cast<int8_t>(h_ptr->_function));
        auto& cd = encoder::encoder_map.at(h_ptr->_function);
        std::vector<char> tmp_bytes;
        // 可变结构
        cd(h_ptr, c_ptr, e_ptr, tmp_bytes);
        // 开始报尾固定结构
        const auto content_len_with_flag = static_cast<uint16_t>(tmp_bytes.size() | 0x8000);
        w.write_byte(static_cast<uint8_t>((content_len_with_flag >> 8) & 0xFF));
        w.write_byte(static_cast<uint8_t>(content_len_with_flag & 0xFF));
        w.write_byte(static_cast<int8_t>(model::const_symbol::stx));
        // 写入可变结构
        w.write_bytes(tmp_bytes.data(), tmp_bytes.size());
        w.write_byte(static_cast<int8_t>(model::const_symbol::esc));
        custom_crc_type crc_calculator;
        crc_calculator.process_bytes(sb->data().data(), sb->size());
        w.write_short(static_cast<int16_t>(crc_calculator.checksum()));
        SPDLOG_DEBUG(" 响应报文 {} ", w.debug());
        return sb;
    }

    size_t decoder::header_max_len() {
        return HEADER_COUNT_LEN;
    }

    size_t decoder::end_length() {
        return END_COUNT_LEN;
    }

    size_t decoder::content_length(const asio::const_buffer& buffer) {
        byte_buf_reader r(buffer);
        short content_length;
        r.get_short(content_length, 11);
        return content_length & 0xfff;
    }

    bool decoder::version_by_buf(const asio::const_buffer& buffer) {
        byte_buf_reader r(buffer);
        int16_t c;
        r.read_short(c);
        return c == static_cast<int16_t>(model::frame_symbol::start_and_end);
    }

    crc_value_t decoder::crc_calculate(const asio::const_buffer& header, const asio::const_buffer& content,
                                       const asio::const_buffer& end) {
        custom_crc_type crc_calculator;
        // 用户数据区的crc校验，需要校验头部加上正文部分
        crc_calculator.process_bytes(header.data(), asio::buffer_size(header));
        crc_calculator.process_bytes(content.data(), asio::buffer_size(content));
        // 这里还要读取尾部一个字节
        crc_calculator.process_bytes(end.data(), 1);
        // sl651_2014 是两字节构成的 short
        return static_cast<int16_t>(crc_calculator.checksum());
    }

    bool decoder::crc_check(const asio::const_buffer& header, const asio::const_buffer& content,
                            const asio::const_buffer& end) {
        auto calc_crc = this->crc_calculate(header, content, end);
        byte_buf_reader end_reader(end);
        end_reader.skip(1);
        int16_t req_crc;
        end_reader.read_short(req_crc);
        // 从请求中读取crc和计算的crc做比较
        return req_crc == std::get<int16_t>(calc_crc);
    }

} // namespace sl651_2014::codec

namespace sl651_2014::parse {

    namespace {

        template <typename T> T& require_optional(std::optional<T>& value, const char* name) {
            if (!value.has_value()) {
                throw sl651_2014::error(std::string("缺少") + name + "数据容器");
            }
            return value.value();
        }

        parse_function
        make_numeric_parser(model::data_type data_type, const char* trace_name,
                            const std::function<std::optional<double>&(model::content&)>& value_accessor) {
            return [data_type, trace_name, value_accessor](byte_buf_reader& reader, byte_buf_reader& hex_reader,
                                                           model::c_shared_ptr& content) {
                auto& value_opt = value_accessor(*content);
                const auto data_len = parse_data(reader, value_opt);
                handle_v(content, hex_reader, data_len, data_type, value_opt);
                if (value_opt.has_value()) {
                    SPDLOG_TRACE("{}  {}", trace_name, value_opt.value());
                }
            };
        }

    } // namespace

    /**
     * 解析 bcd 编码的数据
     */
    uint32_t parse_data(byte_buf_reader& reader, std::optional<double>& v) {
        int8_t data_info;
        // 读取一个字节，包含后续数据长度和小数点位数
        reader.read_byte(data_info);
        const auto data_info_u = static_cast<uint8_t>(data_info);
        // 取高五位 data_length 为数据的长度
        uint32_t data_length = data_info_u >> 3;
        // 小数点取低三位，小数点位置
        uint32_t decimal_point = data_info_u & 0x07;

        if (reader.readable_bytes() < data_length) {
            throw sl651_2014::error("要素数据长度超过剩余可读字节数");
        }

        bool is_invalid_value = data_length > 0;
        const auto data_start_index = reader.read_index();
        for (uint32_t i = 0; i < data_length; ++i) {
            int8_t raw;
            reader.get_byte(raw, data_start_index + i);
            is_invalid_value = is_invalid_value && (static_cast<uint8_t>(raw) == 0xAA);
        }

        if (is_invalid_value) {
            reader.skip(data_length);
            v = std::numeric_limits<double>::quiet_NaN();
            return data_length + 1;
        }

        std::int64_t ret = reader.read_bcd_byte_to_int64(data_length);
        // 除以 10^decimal_point，小数点左移
        v = static_cast<double>(ret) / std::pow(10.0, decimal_point);
        return data_length + 1;
    }

    // 初始化 unordered_map
    const std::unordered_map<model::data_type, parse_function> strategy = {

        /**
         * 观测时间引导符
         */
        {model::data_type::tt,
         [](byte_buf_reader& reader, byte_buf_reader& hex_reader, model::c_shared_ptr& content) {
             auto& back = content->_raw_list.back();
             std::string temp;
             hex_reader.read_hex_str(temp, 1);
             back._raw_hex.append(temp);
             back._length += 1;
             back._v.append(temp);
             // 再读一次，标准里面要求是两个 tt
             int8_t tt = reader.read_byte();
             if (tt != static_cast<int8_t>(model::data_type::tt)) {
                 throw sl651_2014::error("错误的观测时间标识符 ");
             }
             // 临时时间指针，sl651里面明确规定了时间的长度，需要减少重复代码
             boost::posix_time::ptime* tm_ptr = &content->_obs_tm;
             // 观测时间读取五个字节
             reader.read_tm(*tm_ptr, 5);
             // 处理报文取值
             nlohmann::json j;
             to_json(j, *tm_ptr);
             content->_raw_list.emplace_back(hex_reader, OBS_TIME_LEN, OBS_TIME_NOTE, j);
         }},

        /**
         * 测站编码
         */
        {model::data_type::st,
         [](byte_buf_reader& reader, byte_buf_reader& hex_reader, std::shared_ptr<model::content>& content) {
             // 再读一次，标准里面要求是两个 st
             auto& back = content->_raw_list.back();
             std::string temp;
             hex_reader.read_hex_str(temp, 1);
             back._raw_hex.append(temp);
             back._length += 1;
             back._v.append(temp);
             int8_t st_byte = reader.read_byte();
             if (st_byte != static_cast<int8_t>(model::data_type::st)) {
                 throw sl651_2014::error("错误的测站编码标识符 ");
             }
             // 站码 5字节
             reader.read_hex_str(content->_stcd, 1);
             // 需要排除掉第一字节为 00 的情况，例如 0060854433
             // 千万不能用 bcd 方式读取 可能会出现 006085443A
             if (content->_stcd == "00") {
                 content->_stcd.clear();
             }
             reader.read_hex_str(content->_stcd, 4);
             SPDLOG_TRACE("水质站点 STCD  {}", content->_stcd);
             // 处理报文取值
             content->_raw_list.emplace_back(hex_reader, STCD_LEN, STCD_NOTE, content->_stcd);
         }},

        {model::data_type::c, make_numeric_parser(model::data_type::c, "瞬时水温",
                                                  [](model::content& content) -> std::optional<double>& {
                                                      return require_optional(content._obwt, "水温")._wtmp;
                                                  })},

        {model::data_type::vt,
         make_numeric_parser(model::data_type::vt, "电压",
                             [](model::content& content) -> std::optional<double>& { return content._v; })},

        {model::data_type::ph, make_numeric_parser(model::data_type::ph, "ph值",
                                                   [](model::content& content) -> std::optional<double>& {
                                                       return require_optional(content._awqmd, "水质")._ph;
                                                   })},

        {model::data_type::dox, make_numeric_parser(model::data_type::dox, "溶解氧",
                                                    [](model::content& content) -> std::optional<double>& {
                                                        return require_optional(content._awqmd, "水质")._dox;
                                                    })},

        {model::data_type::cond, make_numeric_parser(model::data_type::cond, "电导率",
                                                     [](model::content& content) -> std::optional<double>& {
                                                         return require_optional(content._awqmd, "水质")._cond;
                                                     })},

        {model::data_type::turb, make_numeric_parser(model::data_type::turb, "浊度",
                                                     [](model::content& content) -> std::optional<double>& {
                                                         return require_optional(content._awqmd, "水质")._turb;
                                                     })},

        {model::data_type::codmn, make_numeric_parser(model::data_type::codmn, "高锰酸盐指数",
                                                      [](model::content& content) -> std::optional<double>& {
                                                          return require_optional(content._awqmd, "水质")._codmn;
                                                      })},

        {model::data_type::nh4n, make_numeric_parser(model::data_type::nh4n, "氨氮",
                                                     [](model::content& content) -> std::optional<double>& {
                                                         return require_optional(content._awqmd, "水质")._nh3n;
                                                     })},

        {model::data_type::tp, make_numeric_parser(model::data_type::tp, "总磷",
                                                   [](model::content& content) -> std::optional<double>& {
                                                       return require_optional(content._awqmd, "水质")._tp;
                                                   })},

        {model::data_type::tn, make_numeric_parser(model::data_type::tn, "总氮",
                                                   [](model::content& content) -> std::optional<double>& {
                                                       return require_optional(content._awqmd, "水质")._tn;
                                                   })},

        {model::data_type::toc, make_numeric_parser(model::data_type::toc, "有机总碳",
                                                    [](model::content& content) -> std::optional<double>& {
                                                        return require_optional(content._awqmd, "水质")._toc;
                                                    })},

        {model::data_type::cu, make_numeric_parser(model::data_type::cu, "铜",
                                                   [](model::content& content) -> std::optional<double>& {
                                                       return require_optional(content._awqmd, "水质")._cu;
                                                   })},

        {model::data_type::zn, make_numeric_parser(model::data_type::zn, "锌",
                                                   [](model::content& content) -> std::optional<double>& {
                                                       return require_optional(content._awqmd, "水质")._zn;
                                                   })},

        {model::data_type::pb, make_numeric_parser(model::data_type::pb, "铅",
                                                   [](model::content& content) -> std::optional<double>& {
                                                       return require_optional(content._awqmd, "水质")._pb;
                                                   })},

        {model::data_type::chla, make_numeric_parser(model::data_type::chla, "叶绿素a",
                                                     [](model::content& content) -> std::optional<double>& {
                                                         return require_optional(content._awqmd, "水质")._chla;
                                                     })},

    };

} // namespace sl651_2014::parse

namespace sl651_2014::encoder {

    // 整数 -> BCD
    char int_to_bcd_char(int val) {
        return static_cast<char>(((val / 10) << 4) | (val % 10));
    }

    const std::unordered_map<model::req_type, encoder_fun> encoder_map = {

        {model::req_type::KEEP_ALIVE,
         [](const model::h_shared_ptr& h, const model::c_shared_ptr& c, const model::e_shared_ptr& e,
            std::vector<char>& buffer) {
             // 链路维持报，按照协议标准，什么都不需要做，buffer 应该是个空的
         }},

        // 遥测站定时报
        {model::req_type::HYDROLOGY_PERIODIC,
         [](const model::h_shared_ptr& h, const model::c_shared_ptr& c, const model::e_shared_ptr& e,
            std::vector<char>& buffer) {
             // 写出流水号
             buffer.emplace_back(static_cast<char>((c->_serial_num >> 8) & 0xFF));
             buffer.emplace_back(static_cast<char>(c->_serial_num & 0xFF));
             // 写出接收时间
             auto now = boost::posix_time::second_clock::local_time();
             auto date = now.date();
             auto time = now.time_of_day();
             buffer.emplace_back(int_to_bcd_char(date.year() % 100));
             buffer.emplace_back(int_to_bcd_char(date.month()));
             buffer.emplace_back(int_to_bcd_char(date.day()));
             buffer.emplace_back(int_to_bcd_char(time.hours()));
             buffer.emplace_back(int_to_bcd_char(time.minutes()));
             buffer.emplace_back(int_to_bcd_char(time.seconds()));
             // 写出测站
             buffer.emplace_back(static_cast<char>(model::data_type::st));
             std::string rtu_stcd = h->_rtu_stcd;
             if (rtu_stcd.length() == 8) {
                 // 如果长度是 8，那么就要在前面补充两个 0，得到 10位
                 rtu_stcd.insert(0, 2, '0');
             }
             // 直接编码测站
             boost::algorithm::unhex(rtu_stcd.begin(), rtu_stcd.end(), std::back_inserter(buffer));
         }},

        // 遥测站定时报
        {model::req_type::REGULAR,
         [](const model::h_shared_ptr& h, const model::c_shared_ptr& c, const model::e_shared_ptr& e,
            std::vector<char>& buffer) {
             // 默认用均匀时段水文信息报
             auto& def_decoder = encoder_map.at(model::req_type::HYDROLOGY_PERIODIC);
             def_decoder(h, c, e, buffer);
         }},

        // 遥测站加报报
        {model::req_type::ADDITIONAL,
         [](const model::h_shared_ptr& h, const model::c_shared_ptr& c, const model::e_shared_ptr& e,
            std::vector<char>& buffer) {
             // 默认用均匀时段水文信息报
             auto& def_decoder = encoder_map.at(model::req_type::HYDROLOGY_PERIODIC);
             def_decoder(h, c, e, buffer);
         }},

        // 遥测站小时报
        {model::req_type::HOURLY,
         [](const model::h_shared_ptr& h, const model::c_shared_ptr& c, const model::e_shared_ptr& e,
            std::vector<char>& buffer) {
             // 默认用均匀时段水文信息报
             auto& def_decoder = encoder_map.at(model::req_type::HYDROLOGY_PERIODIC);
             def_decoder(h, c, e, buffer);
         }},

        // 遥测站人工置数报
        {model::req_type::MANUAL_DATA, [](const model::h_shared_ptr& h, const model::c_shared_ptr& c,
                                          const model::e_shared_ptr& e, std::vector<char>& buffer) {
             // 默认用均匀时段水文信息报
             auto& def_decoder = encoder_map.at(model::req_type::HYDROLOGY_PERIODIC);
             def_decoder(h, c, e, buffer);
         }}};
} // namespace sl651_2014::encoder
