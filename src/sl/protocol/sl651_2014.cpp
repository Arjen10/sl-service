//
// Created by Arjen on 2025/9/12.
//

#include "sl651_2014.hpp"

namespace sl651_2014::json {

    void to_json(nlohmann::json &j, const model::req_type &type) {
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

    void to_json(nlohmann::json &j, const model::station_type &type) {
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

    void to_json(nlohmann::json &j, const model::data_type &type) {
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

}

namespace sl651_2014 {

    error::error(const std::string &msg) {
        this->full_msg.append("sl651_2014解析异常 ----> ").append(msg);
    }

    const char *error::what() const _NOEXCEPT {
        return full_msg.c_str();
    }
}

namespace sl651_2014::model {

    void to_json(nlohmann::json &j, const h_shared_ptr &h) {
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

    void to_json(nlohmann::json &j, const c_shared_ptr &c) {
        j = nlohmann::json::object();
        JSON_FIELD_PTR(j, c, _serial_num);
        JSON_FIELD_PTR(j, c, _report_tm);
        JSON_FIELD_PTR(j, c, _stcd);
        JSON_FIELD_PTR(j, c, _station_type);
        JSON_FIELD_PTR(j, c, _v);
        JSON_FIELD_PTR(j, c, _awqmd);
        JSON_FIELD_PTR(j, c, _raw_list);
    }

    void to_json(nlohmann::json &j, const e_shared_ptr &e) {
        j = nlohmann::json::object();
        JSON_FIELD_PTR(j, e, _end_note);
        JSON_FIELD_PTR(j, e, _crc);
        JSON_FIELD_PTR(j, e, _raw_list);
    }

    void content::init(const station_type &station_type, c_shared_ptr &c) {
        this->_station_type = station_type;
        switch (station_type) {
            case wq:
                this->_awqmd.emplace(c->_stcd);
                return;
            case rr:
            case pp:
                // todo 这里需要初始化不同的类，避免空引用
            case zq_zz:
            default:
                throw sl651_2014::error("获取测站类型错误！");
        }
    }

    std::unordered_map<std::string, std::optional<double>> &content::expand_map() {
        switch (this->_station_type) {
            case wq:
                return this->_awqmd->_extended;
            case rr:
            case pp:
            case zq_zz:
            default:
                throw sl651_2014::error("获取测站类型错误！");
        }
    }

}

namespace sl651_2014::codec {

    std::string decoder::protocol_name() {
        static const std::string protocol_name = "SL651-2014";
        return protocol_name;
    }

    model::h_shared_ptr decoder::parse_header(const asio::const_buffer &hb) {
        auto h_ptr = std::make_shared<model::header>();
        byte_buf_reader reader(hb);
        byte_buf_reader hex_reader(hb);
        // 读取版本字节
        reader.skip(VERSION_CONTROL_LEN);
        h_ptr->_raw_list.emplace_back(hex_reader,
                                      VERSION_CONTROL_LEN,
                                      VERSION_CONTROL_NOTE, this->protocol_name());
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
            LOG_DEBUG << "这是一个上行数据";
        }
        h_ptr->_content_length = this->content_length(hb);
        std::stringstream ss;
        ss << "当前报文类型为:" << (h_ptr->_is_up ? "上" : "下") << "行报文, 报文正文长度为: "
           << h_ptr->_content_length;
        // 报文上行标识符及长度
        h_ptr->_raw_list.emplace_back(hex_reader, REQ_LEN, REQ_NOTE, ss.str());
        // 报文起始帧 1字节
        h_ptr->_raw_list.emplace_back(hex_reader, MES_START_LEN, MES_START_NOTE);
        reader.read_byte(h_ptr->_start_character);
        if (model::const_symbol::stx != h_ptr->_start_character) {
            std::string what = std::string("错误的帧起始符 ");
            what.append(reinterpret_cast<const char *>(&h_ptr->_start_character), 1);
            throw sl651_2014::error(what);
        }
        // 报头正确性检验
        if (reader.readable_bytes() > 0) {
            throw sl651_2014::error("错误的头部帧");
        }
        return h_ptr;
    }

    model::c_shared_ptr decoder::parse_content(const model::h_shared_ptr &h_ptr,
                                               const asio::const_buffer &cb) {
        auto c_ptr = std::make_shared<model::content>();
        byte_buf_reader reader(cb);
        byte_buf_reader hex_reader(cb);
        // 流水号 2字节
        reader.read_short(c_ptr->_serial_num);
        c_ptr->_raw_list.emplace_back(hex_reader, SERIAL_NUM_LEN, SERIAL_NUM_NOTE,
                                      std::to_string(c_ptr->_serial_num));
        // 发报时间 6字节 250721013101
        reader.read_tm(c_ptr->_report_tm, REPORT_TM_LEN);
        nlohmann::json temp_j;
        to_json(temp_j, c_ptr->_report_tm);
        LOG_DEBUG << " 发报时间 " << temp_j.get<std::string>();
        c_ptr->_raw_list.emplace_back(hex_reader, REPORT_TM_LEN, REPORT_TM_NOTE, temp_j);
        // 处理链路维持报，当可读的字节等于 0 的时候，就是设备发的心跳包
        if (reader.readable_bytes() == 0) {
            if (h_ptr->_function != model::req_type::KEEP_ALIVE) {
                throw sl651_2014::error("错误的帧和功能码！");
            }
            LOG_DEBUG << "设备心跳包";
            return nullptr;
        }
        // 测站编码引导符 2字节
        int8_t stcd_lead_symbol = reader.read_byte();
        c_ptr->_raw_list.emplace_back(hex_reader, 1, STCD_LEAD_SYMBOL_NOTE);
        if (stcd_lead_symbol != static_cast<int8_t>(model::data_type::st)) {
            throw sl651_2014::error("获取测站编码错误！");
        }
        const parse::parse_function &st_func = parse::strategy.at(model::data_type::st);
        st_func(reader, hex_reader, c_ptr);

        // 开始处理测站类型 1个字节
        int8_t station_type_lead_symbol = reader.read_byte();
        auto st = static_cast<model::station_type>(station_type_lead_symbol);
        // 执行初始化
        c_ptr->init(st, c_ptr);
        temp_j.clear();
        json::to_json(temp_j, st);
        c_ptr->_raw_list.emplace_back(hex_reader, STATION_TYPE_LEN, STATION_TYPE_NOTE, temp_j);
        std::optional<double> temp_opt;
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
            if (parse::strategy.count(key) != 0) {
                temp_j.clear();
                json::to_json(temp_j, key);
                c_ptr->_raw_list.emplace_back(hex_reader, 1, temp_j.get<std::string>() + "标识符");
                const parse::parse_function &func = parse::strategy.at(key);
                func(reader, hex_reader, c_ptr);
                continue;
            }
            // 如果不是标准值，解析后返回给业务端
            temp_opt.reset();
            auto data_len = parse::parse_data(reader, temp_opt);
            try {
                std::ostringstream oss;
                oss << std::setw(2) << std::setfill('0') << std::hex << (int32_t) element_lead_symbol;
                auto &expand_map = c_ptr->expand_map();
                // 这里存入 hex 编码
                expand_map.emplace(oss.str(), temp_opt);
                std::string expand_note = "拓展标识符 " + oss.str();
                c_ptr->_raw_list.emplace_back(hex_reader, 1, expand_note, true);
                auto v_str = temp_opt ? std::to_string(temp_opt.value()) : "null";
                c_ptr->_raw_list.emplace_back(hex_reader, data_len, expand_note + "  取值", v_str, true);
                LOG_DEBUG << " 不支持的标识符 " << oss.str() << " 值 " << v_str;
            } catch (std::exception &e) {
                LOG_ERROR << e.what();
                throw e;
            }
        }
    }

    model::e_shared_ptr decoder::parse_end(const model::h_shared_ptr &h_ptr,
                                           const model::c_shared_ptr &c_ptr,
                                           const asio::const_buffer &eb) {
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
        end_ptr->_raw_list.emplace_back(hex_reader, END_CRC_LEN, END_CRC_NOTE,
                                        std::to_string(end_ptr->_crc));
        return end_ptr;
    }

    void decoder::do_something(const model::h_shared_ptr &h_ptr,
                               const model::c_shared_ptr &c_ptr,
                               const model::e_shared_ptr &e_ptr) {
        this->call_back(h_ptr->_rtu_stcd, h_ptr, c_ptr, e_ptr);
    }

    std::optional<std::shared_ptr<asio::streambuf>> decoder::resp_byte_buffer(const model::h_shared_ptr &h_ptr,
                                                                              const model::c_shared_ptr &c_ptr,
                                                                              const model::e_shared_ptr &e_ptr) {
        // 协议标准说明，链路维持报，无需响应
        if (h_ptr->_function == model::req_type::KEEP_ALIVE) {
            return std::nullopt;
        }
        auto sb = std::make_shared<asio::streambuf>();
        std::ostream os(&(*sb));
        // --------开始报头部分----------
        // 写出报头
        short head = model::frame_symbol::start_and_end;
        os.write(reinterpret_cast<char *>(&head), sizeof(head));
        std::string rtu_stcd = (h_ptr->_rtu_stcd.length() == 10 ? h_ptr->_rtu_stcd : ("00" + h_ptr->_rtu_stcd));
        // rtu stcd
        std::vector<char> tmp_bytes;
        boost::algorithm::unhex(rtu_stcd.begin(), rtu_stcd.end(), std::back_inserter(tmp_bytes));
        os.write(tmp_bytes.data(), tmp_bytes.size());
        // 中心站
        os.put(h_ptr->_central_address);
        tmp_bytes.clear();
        // 密码
        boost::algorithm::unhex(h_ptr->_pwd.begin(), h_ptr->_pwd.end(), std::back_inserter(tmp_bytes));
        os.write(tmp_bytes.data(), tmp_bytes.size());
        // 功能码 1 字节
        os.put(h_ptr->_function);
        auto &cd = encoder::encoder_map.at(h_ptr->_function);
        // 得到正文部分
        cd(h_ptr, c_ptr, e_ptr, tmp_bytes);
        // 这里写入正文长度
        os.put(static_cast<short>(tmp_bytes.size() | 0x8000));
        // 写出正文
        os.put(model::const_symbol::stx);
        os.write(tmp_bytes.data(), tmp_bytes.size());
        tmp_bytes.clear();
        // --------开始报尾部分----------
        os.put(model::const_symbol::esc);
        // crc计算
        custom_crc_type crc_calculator;
        crc_calculator.process_bytes(sb->data().data(), sb->size());
        // 写出 crc
        auto ret_crc = static_cast<short>(crc_calculator.checksum());
        os.write(reinterpret_cast<const char *>(&ret_crc), sizeof(ret_crc));
        return sb;
    }

    size_t decoder::header_max_len() {
        return HEADER_COUNT_LEN;
    }

    size_t decoder::end_length() {
        return END_COUNT_LEN;
    }

    size_t decoder::content_length(const asio::const_buffer &buffer) {
        byte_buf_reader r(buffer);
        short content_length;
        r.get_short(content_length, 11);
        return content_length & 0xfff;
    }

    bool decoder::version_by_buf(const asio::const_buffer &buffer) {
        byte_buf_reader r(buffer);
        int16_t c;
        r.read_short(c);
        return c == static_cast<int16_t>(model::frame_symbol::start_and_end);
    }

    crc_value_t decoder::crc_calculate(const asio::const_buffer &header,
                                       const asio::const_buffer &content,
                                       const asio::const_buffer &end) {
        custom_crc_type crc_calculator;
        // 用户数据区的crc校验，需要校验头部加上正文部分
        crc_calculator.process_bytes(header.data(), asio::buffer_size(header));
        crc_calculator.process_bytes(content.data(), asio::buffer_size(content));
        // 这里还要读取尾部一个字节
        crc_calculator.process_bytes(end.data(), 1);
        // sl651_2014 是两字节构成的 short
        return static_cast<int16_t>(crc_calculator.checksum());
    }

    bool decoder::crc_check(const asio::const_buffer &header,
                            const asio::const_buffer &content,
                            const asio::const_buffer &end) {
        auto calc_crc = this->crc_calculate(header, content, end);
        byte_buf_reader end_reader(end);
        end_reader.skip(1);
        int16_t req_crc;
        end_reader.read_short(req_crc);
        // 从请求中读取crc和计算的crc做比较
        return req_crc == std::get<int16_t>(calc_crc);
    }

}

namespace sl651_2014::parse {

    /**
     * 解析 bcd 编码的数据
     */
    uint32_t parse_data(byte_buf_reader &reader, std::optional<double> &v) {
        int8_t data_info;
        // 读取一个字节，包含后续数据长度和小数点位数
        reader.read_byte(data_info);
        // 取高五位 data_length 为数据的长度
        uint32_t data_length = data_info >> 3;
        // 小数点取低三位，小数点位置
        uint32_t decimal_point = data_info & 0x07;
        std::int64_t ret = reader.read_bcd_byte_to_int64(data_length);
        // 0xAAAAAAAA 无效值
        if (ret == 0xAAAAAAAAULL) {
            v = std::numeric_limits<double>::quiet_NaN();
        } else {
            // 除以 10^decimal_point，小数点左移
            v = static_cast<double>(ret) / std::pow(10.0, decimal_point);
        }
        return data_length + 1;
    }

    // 初始化 unordered_map
    const std::unordered_map<model::data_type, parse_function> strategy = {

            /**
             * 观测时间引导符
             */
            {model::data_type::tt,    [](byte_buf_reader &reader,
                                         byte_buf_reader &hex_reader,
                                         std::shared_ptr<model::content> &content) {
                auto &back = content->_raw_list.back();
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
                boost::posix_time::ptime *tm_ptr = nullptr;
                switch (content->_station_type) {
                    case model::station_type::wq:
                        tm_ptr = &content->_awqmd->_spt;
                        break;
                    default:
                        throw sl651_2014::error("未支持的测站类型");
                }
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
            {model::data_type::st,    [](byte_buf_reader &reader,
                                         byte_buf_reader &hex_reader,
                                         std::shared_ptr<model::content> &content) {
                // 再读一次，标准里面要求是两个 st
                auto &back = content->_raw_list.back();
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
                LOG_DEBUG << " 水质站点 STCD " << content->_stcd;
                // 处理报文取值
                content->_raw_list.emplace_back(hex_reader, STCD_LEN, STCD_NOTE, content->_stcd);
            }},

            /**
             * 瞬时水温
             */
            {model::data_type::c,     [](byte_buf_reader &reader,
                                         byte_buf_reader &hex_reader,
                                         std::shared_ptr<model::content> &content) {
                auto &wt_opt = content->_awqmd->_wt;
                uint32_t data_len = parse_data(reader, wt_opt);
                handle_v(content, hex_reader, data_len, model::data_type::c, wt_opt);
                if (wt_opt.has_value()) {
                    LOG_DEBUG << " 瞬时水温 " << wt_opt.value();
                }
            }},

            /**
             * 电压
             */
            {model::data_type::vt,    [](byte_buf_reader &reader,
                                         byte_buf_reader &hex_reader,
                                         std::shared_ptr<model::content> &content) {
                auto &v_opt = content->_v;
                auto data_len = parse_data(reader, v_opt);
                handle_v(content, hex_reader, data_len, model::data_type::vt, v_opt);
                if (v_opt.has_value()) {
                    LOG_DEBUG << " 电压 " << v_opt.value();
                }
            }},

            /**
             * ph值
             */
            {model::data_type::ph,    [](byte_buf_reader &reader,
                                         byte_buf_reader &hex_reader,
                                         std::shared_ptr<model::content> &content) {
                auto &ph_opt = content->_awqmd->_ph;
                auto data_len = parse_data(reader, ph_opt);
                handle_v(content, hex_reader, data_len, model::data_type::ph, ph_opt);
                if (ph_opt.has_value()) {
                    LOG_DEBUG << " ph值 " << ph_opt.value();
                }
            }},

            /**
             * 溶解氧
             */
            {model::data_type::dox,   [](byte_buf_reader &reader,
                                         byte_buf_reader &hex_reader,
                                         std::shared_ptr<model::content> &content) {
                auto &dox_opt = content->_awqmd->_dox;
                auto data_len = parse_data(reader, dox_opt);
                handle_v(content, hex_reader, data_len, model::data_type::dox, dox_opt);
                if (dox_opt.has_value()) {
                    LOG_DEBUG << " 溶解氧 " << dox_opt.value();
                }
            }},

            /**
             * 电导率
             */
            {model::data_type::cond,  [](byte_buf_reader &reader,
                                         byte_buf_reader &hex_reader,
                                         std::shared_ptr<model::content> &content) {
                auto &cond_opt = content->_awqmd->_cond;
                auto data_len = parse_data(reader, cond_opt);
                handle_v(content, hex_reader, data_len, model::data_type::cond, cond_opt);
                if (cond_opt.has_value()) {
                    LOG_DEBUG << " 电导率 " << cond_opt.value();
                }
            }},

            /**
             * 浊度
             */
            {model::data_type::turb,  [](byte_buf_reader &reader,
                                         byte_buf_reader &hex_reader,
                                         std::shared_ptr<model::content> &content) {
                auto &turb_opt = content->_awqmd->_turb;
                auto data_len = parse_data(reader, turb_opt);
                handle_v(content, hex_reader, data_len, model::data_type::turb, turb_opt);
                if (turb_opt.has_value()) {
                    LOG_DEBUG << " 浊度 " << turb_opt.value();
                }
            }},

            /**
             * 叶绿素a
             */
            {model::data_type::chla,  [](byte_buf_reader &reader,
                                         byte_buf_reader &hex_reader,
                                         std::shared_ptr<model::content> &content) {
                auto &chla_opt = content->_awqmd->_chla;
                auto data_len = parse_data(reader, chla_opt);
                handle_v(content, hex_reader, data_len, model::data_type::chla, chla_opt);
                if (chla_opt.has_value()) {
                    LOG_DEBUG << " 叶绿素a " << chla_opt.value();
                }
            }},

            /**
             * 高锰酸盐浓度
             */
            {model::data_type::codmn, [](byte_buf_reader &reader,
                                         byte_buf_reader &hex_reader,
                                         std::shared_ptr<model::content> &content) {
                auto &codmn_opt = content->_awqmd->_codmn;
                auto data_len = parse_data(reader, codmn_opt);
                handle_v(content, hex_reader, data_len, model::data_type::codmn, codmn_opt);
                if (codmn_opt.has_value()) {
                    LOG_DEBUG << " 高锰酸盐浓度 " << codmn_opt.value();
                }
            }},

            /**
             * 氨氮
             */
            {model::data_type::nh4n,  [](byte_buf_reader &reader,
                                         byte_buf_reader &hex_reader,
                                         std::shared_ptr<model::content> &content) {
                // 这里是按照标准建表
                auto &nh3n_opt = content->_awqmd->_nh3n;
                auto data_len = parse_data(reader, nh3n_opt);
                handle_v(content, hex_reader, data_len, model::data_type::nh4n, nh3n_opt);
                if (nh3n_opt.has_value()) {
                    LOG_DEBUG << " 氨氮 " << nh3n_opt.value();
                }
            }},

            /**
             * 总磷
             */
            {model::data_type::tp,    [](byte_buf_reader &reader,
                                         byte_buf_reader &hex_reader,
                                         std::shared_ptr<model::content> &content) {
                // 这里是按照标准建表
                auto &tp_opt = content->_awqmd->_tp;
                auto data_len = parse_data(reader, tp_opt);
                handle_v(content, hex_reader, data_len, model::data_type::tp, tp_opt);
                if (tp_opt.has_value()) {
                    LOG_DEBUG << " 总磷 " << tp_opt.value();
                }
            }},

            /**
             * 总磷
             */
            {model::data_type::tn,    [](byte_buf_reader &reader,
                                         byte_buf_reader &hex_reader,
                                         std::shared_ptr<model::content> &content) {
                // 这里是按照标准建表
                auto &tn_opt = content->_awqmd->_tn;
                auto data_len = parse_data(reader, tn_opt);
                handle_v(content, hex_reader, data_len, model::data_type::tn, tn_opt);
                if (tn_opt.has_value()) {
                    LOG_DEBUG << " 总氮 " << tn_opt.value();
                }
            }},

    };

}

namespace sl651_2014::encoder {

    // 整数 -> BCD
    char int_to_bcd_char(int val) {
        return static_cast<char>(((val / 10) << 4) | (val % 10));
    }

    const std::unordered_map<model::req_type, encoder_fun> encoder_map = {

            {model::req_type::KEEP_ALIVE,         [](const model::h_shared_ptr &h,
                                                     const model::c_shared_ptr &c,
                                                     const model::e_shared_ptr &e,
                                                     std::vector<char> &buffer) {
                // 链路维持报，按照协议标准，什么都不需要做，buffer 应该是个空的
            }},

            // 遥测站定时报
            {model::req_type::HYDROLOGY_PERIODIC, [](const model::h_shared_ptr &h,
                                                     const model::c_shared_ptr &c,
                                                     const model::e_shared_ptr &e,
                                                     std::vector<char> &buffer) {
                // 写出流水号
                buffer.emplace_back(c->_serial_num);
                // 写出接收时间
                auto now = boost::posix_time::second_clock::local_time();
                auto date = now.date();
                auto time = now.time_of_day();
                buffer.emplace_back(int_to_bcd_char(date.year()));
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
            {model::req_type::REGULAR,            [](const model::h_shared_ptr &h,
                                                     const model::c_shared_ptr &c,
                                                     const model::e_shared_ptr &e,
                                                     std::vector<char> &buffer) {
                // 默认用均匀时段水文信息报
                auto &def_decoder = encoder_map.at(model::req_type::HYDROLOGY_PERIODIC);
                def_decoder(h, c, e, buffer);
            }},

            // 遥测站加报报
            {model::req_type::ADDITIONAL,         [](const model::h_shared_ptr &h,
                                                     const model::c_shared_ptr &c,
                                                     const model::e_shared_ptr &e,
                                                     std::vector<char> &buffer) {
                // 默认用均匀时段水文信息报
                auto &def_decoder = encoder_map.at(model::req_type::HYDROLOGY_PERIODIC);
                def_decoder(h, c, e, buffer);
            }},

            // 遥测站小时报
            {model::req_type::HOURLY,             [](const model::h_shared_ptr &h,
                                                     const model::c_shared_ptr &c,
                                                     const model::e_shared_ptr &e,
                                                     std::vector<char> &buffer) {
                // 默认用均匀时段水文信息报
                auto &def_decoder = encoder_map.at(model::req_type::HYDROLOGY_PERIODIC);
                def_decoder(h, c, e, buffer);
            }},

            // 遥测站人工置数报
            {model::req_type::MANUAL_DATA,        [](const model::h_shared_ptr &h,
                                                     const model::c_shared_ptr &c,
                                                     const model::e_shared_ptr &e,
                                                     std::vector<char> &buffer) {
                // 默认用均匀时段水文信息报
                auto &def_decoder = encoder_map.at(model::req_type::HYDROLOGY_PERIODIC);
                def_decoder(h, c, e, buffer);
            }}
    };
}

