//
// Created by Arjen on 2025/9/12.
//

#ifndef SL_SERVICE_SL651_2014_HPP
#define SL_SERVICE_SL651_2014_HPP

#include <boost/crc.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <nlohmann/json.hpp>

#include "../basic_decoder.hpp"
#include "../gb/slt324_2019.hpp"
#include "../../core/json_ser_deser.hpp"
#include "../../core/except.hpp"

// 651协议头，用于确定协议类型
#define SL651_2014_VER_HEX 0x7e7e

// -------------M1/2/4 上行帧头部定义 ---------------
#define VERSION_CONTROL_LEN 2
#define VERSION_CONTROL_NOTE "版本字节"

#define CENTRAL_ADDR_LEN 1
#define CENTRAL_ADDR_NOTE "中心站址"

#define RTU_STCD_LEN 5
#define RTU_STCD_NOTE "遥测站地址"

#define PWD_LEN 2
#define PWD_NOTE "密码"

#define FUNCTION_LEN 1
#define FUNCTION_NOTE "功能码"

#define REQ_LEN 2
#define REQ_NOTE "报文上行标识符及长度"

#define MES_START_LEN 1
#define MES_START_NOTE "报文起始帧"

// 头部总计长度
#define HEADER_COUNT_LEN 14

// -------------上行帧正文部定义 ---------------

#define SERIAL_NUM_LEN 2
#define SERIAL_NUM_NOTE "流水号"

#define REPORT_TM_LEN 6
#define REPORT_TM_NOTE "发报时间"

#define STCD_LEAD_SYMBOL_LEN 2
#define STCD_LEAD_SYMBOL_NOTE "测站编码引导符"

#define STCD_LEN RTU_STCD_LEN
#define STCD_NOTE RTU_STCD_NOTE

#define STATION_TYPE_LEN 1
#define STATION_TYPE_NOTE "测站分类码"

#define OBS_TIME_SYMBOL_LEN 2
#define OBS_TIME_SYMBOL_NOTE "观测时间标识符"

#define OBS_TIME_LEN 5
#define OBS_TIME_NOTE "观测时间"

// ------------M1/2/4 上行帧结束部分------------------
#define END_LEN 1
#define END_NOTE "报文结束符"

#define END_CRC_LEN 2
#define END_CRC_NOTE "校验码"

// 结束部分总计 3 个字节
#define END_COUNT_LEN 3

// -------------附录C要素定义---------------

namespace sl651_2014 {

    /**
     * 解析异常类
     */
    class error : public parse::error {
      private:
        std::string full_msg;

      public:
        error(const std::string& msg);
        const char* what() const noexcept override;
    };

    namespace model {

        class header;

        using h_shared_ptr = std::shared_ptr<header>;

        class content;

        using c_shared_ptr = std::shared_ptr<content>;

        class end;

        using e_shared_ptr = std::shared_ptr<end>;
    } // namespace model

    namespace codec {

        constexpr std::size_t CRC_POLYNOMIAL = 0x8005;        // 多项式
        constexpr std::size_t CRC_INITIAL_REMAINDER = 0xFFFF; // 初始值
        constexpr std::size_t CRC_FINAL_XOR_VALUE = 0x0;      // 结果异或值
        constexpr bool CRC_REFLECT_INPUT = true;              // 反转输入
        constexpr bool CRC_REFLECT_OUTPUT = true;             // 反转输出

        // crc 计算公式
        using custom_crc_type = boost::crc_optimal<16, CRC_POLYNOMIAL, CRC_INITIAL_REMAINDER, CRC_FINAL_XOR_VALUE,
                                                   CRC_REFLECT_INPUT, CRC_REFLECT_OUTPUT>;

        class decoder : public sl_decoder_crtp<codec::decoder, model::header, model::content, model::end> {

          private:
          public:
            bool crc_check(const asio::const_buffer& header, const asio::const_buffer& content,
                           const asio::const_buffer& end) override;

            crc_value_t crc_calculate(const asio::const_buffer& header, const asio::const_buffer& content,
                                      const asio::const_buffer& end) override;

            std::string protocol_name() override;

            /**
             * HEX 结构，M1/M2/M4 上行头部帧共计 14 字节
             */
            model::h_shared_ptr parse_header(const asio::const_buffer& hb);

            model::c_shared_ptr parse_content(const model::h_shared_ptr& h_ptr, const asio::const_buffer& cb);

            model::e_shared_ptr parse_end(const model::h_shared_ptr& h_ptr, const model::c_shared_ptr& c_ptr,
                                          const asio::const_buffer& eb);

            void do_something(const model::h_shared_ptr& h_ptr, const model::c_shared_ptr& c_ptr,
                              const model::e_shared_ptr& e_ptr);

            /**
             * 生成响应报文
             * @param h_ptr
             * @param c
             * @param e
             * @param sb
             */
            std::optional<std::shared_ptr<asio::streambuf>> resp_byte_buffer(const model::h_shared_ptr& h_ptr,
                                                                             const model::c_shared_ptr& c_ptr,
                                                                             const model::e_shared_ptr& e_ptr);

            size_t header_max_len() override;

            size_t end_length() override;

            size_t content_length(const boost::asio::const_buffer& buffer) override;

            bool version_by_buf(const boost::asio::const_buffer& buffer) override;
        };
    } // namespace codec

    namespace model {

        /**
         * 请求类型
         */
        enum req_type : char {

            // 遥测站人工置数报
            MANUAL_DATA = 0x35,

            // 链路维持报
            KEEP_ALIVE = 0x2F,

            // 均匀时段水文信息报
            HYDROLOGY_PERIODIC = 0x31,

            // 遥测站定时报
            REGULAR = 0x32,

            // 遥测站加报报
            ADDITIONAL = 0x33,

            // 遥测站小时报
            HOURLY = 0x34,

        };

        /**
         * 帧符号
         */
        enum frame_symbol : short {

            // 帧起始和报文正文起始符
            start_and_end = 0x7E7E,

        };

        /**
         * 表10 报文帧定义符号
         */
        enum const_symbol : char {

            // 报文起始符，传输开始
            stx = 0x02,

            // 作为报文结束符，表示传输完成，等待退出通信
            etx = 0x03,

            // 在报文分包传输时作为报文结束符，表示传输未完成，不可退出通信
            etb = 0x17,

            // 在下行确认帧代替 EOT 作为报文结束符，要求终端在线。保持在线
            // 10分钟内若没有收到中心站命令，终端退出返回原先设定的工作状态
            esc = 0x1B,

        };

        /**
         * 测站类型
         */
        enum station_type : int8_t {

            // 水库水文站
            rr = 0x4B,

            // 雨量站
            pp = 0x50,

            // 水质站
            wq = 0x51,

            // 河道水文站或者河道水位站
            zq_zz = 0x48,

        };

        /**
         * 数据类型标识符，都用带符号为的字节处理，不要管他是正数还是负数，一个字，干就完事儿
         */
        enum data_type : int8_t {

            // 观测时间标识符
            tt = static_cast<int8_t>(0xF0),

            // 测站编码引导符
            st = static_cast<int8_t>(0xF1),

            // 瞬时水温
            c = 0x03,

            // 时间步长码，未测试
            drxnn = 0x04,

            // 1小时内 5 分钟时段雨量
            drp = static_cast<int8_t>(0xF4),

            // 1h内5min间隔相对水位1
            // (每组水位占2字节HEX，分辨率为厘米，最大值为655.34m，数据中不含小数点;FFFFH表示非法数据);
            // 对于河道、闸坝()站分别表示河道水位、闸(站)上水位
            drz1 = static_cast<int8_t>(0xF5),

            // 电压
            vt = 0x38,

            // ph 值
            ph = 0x46,

            // 溶解氧
            dox = 0x47,

            // 电导率
            cond = 0x48,

            // 浊度
            turb = 0x49,

            // 高锰酸盐指数
            codmn = 0x4A,

            // 氧还原电位，未测试
            redox = 0x4B,

            // 氨氮
            nh4n = 0x4C,

            // 总磷
            tp = 0x4D,

            // 总氮
            tn = 0x4E,

            // 有机总碳
            toc = 0x4F,

            // 铜，未测试
            cu = 0x50,

            // 锌，未测试
            zn = 0x51,

            // 硒，未测试
            se = 0x52,

            // 砷，未测试
            as = 0x53,

            // 总汞，未测试
            thg = 0x54,

            // 镉，未测试
            cd = 0x55,

            // 铅，未测试
            pb = 0x56,

            // 叶绿素
            chla = 0x57,

        };

        /**
         * 头部信息
         */
        class header : public base_decoded_result {

            friend codec::decoder;

          public:
            /**
             * 中心地址
             */
            int8_t _central_address;

            /**
             * 遥测站地址
             */
            std::string _rtu_stcd;

            // 密码
            std::string _pwd;

            // 功能码
            req_type _function;

            // 上行或者下行, true是上行
            bool _is_up;

            // 内容长度
            std::size_t _content_length;

            // 起始字符
            int8_t _start_character;

          public:
            friend void to_json(nlohmann::json& j, const std::shared_ptr<header>& h);
        };

        /**
         * 正文信息
         */
        class content : public base_decoded_result {

            friend codec::decoder;

          public:
            /**
             * 流水号
             */
            int16_t _serial_num;

            /**
             * 数据上报时间。此时间一般要比观测时间晚一点，取决于设备的性能
             */
            boost::posix_time::ptime _report_tm;

            /**
             * 观测时间
             */
            boost::posix_time::ptime _obs_tm;

            /**
             * 测站编码
             */
            std::string _stcd;

            /**
             * 测站类型
             */
            station_type _station_type;

            /**
             * 电压
             */
            std::optional<std::double_t> _v;

            /**
             * 水质数据封装
             */
            std::optional<hyd::ri::wqamd_w> _awqmd;

            /**
             * 水温数据封装
             */
            std::optional<hyd::ri::obwt_w> _obwt;

            /**
             * 非标准表字段，这里只是拓展属性，交给业务端处理
             */
            std::unordered_map<std::string, std::optional<double>> _extended;

          private:
            /**
             * 初始化
             */
            void init(const c_shared_ptr& c);

          public:
            friend void to_json(nlohmann::json& j, const std::shared_ptr<content>& c);
        };

        /**
         * 结束部分
         */
        class end : public base_decoded_result {
            friend codec::decoder;

          private:
            /**
             * 结束符
             */
            int8_t _end_note;

            /**
             * crc 校验符
             */
            int16_t _crc;

          public:
            friend void to_json(nlohmann::json& j, const std::shared_ptr<end>& e);
        };

    } // namespace model

    namespace json {

        /**
         * 请求类型转 json
         */
        void to_json(nlohmann::json& j, const model::req_type& type);

        /**
         * 站点类型转 json
         */
        void to_json(nlohmann::json& j, const model::station_type& type);

        /**
         * data_type 转 json
         */
        void to_json(nlohmann::json& j, const model::data_type& type);

    } // namespace json

    /**
     * 解析器
     */
    namespace parse {

        using parse_function = std::function<void(byte_buf_reader& reader, byte_buf_reader& hex_reader,
                                                  std::shared_ptr<model::content>& content)>;

        /**
         * 解析数据
         * @param reader 字节读取器
         * @param v 数据引用
         * @return 读取的长度
         */
        uint32_t parse_data(byte_buf_reader& reader, std::optional<double>& v);

        /**
         * 存放标识符的解析函数
         */
        extern const std::unordered_map<model::data_type, parse_function> strategy;

        /**
         * 处理原始报文的值
         * @tparam T 值的类型
         * @param content 正文对象
         * @param hex_reader 报文读取器
         * @param data_len 数据长度
         * @param data_type 数据类型
         * @param opt 值
         */
        template <typename T>
        void handle_v(std::shared_ptr<sl651_2014::model::content>& content, byte_buf_reader& hex_reader,
                      uint32_t data_len, model::data_type data_type, const std::optional<T>& opt) {
            nlohmann::json j;
            json::to_json(j, data_type);
            auto str = j.get<std::string>() + "取值";
            std::string v_str;
            if (opt) {
                T v = *opt;
                // 如果是 double
                if (std::is_same<T, double>::value) {
                    std::ostringstream oss;
                    oss << std::fixed << std::setprecision(3) << v;
                    v_str = oss.str();
                } else {
                    v_str = std::to_string(v);
                }
            } else {
                v_str = "";
            }
            content->_raw_list.emplace_back(hex_reader, data_len, str, v_str);
        }
    } // namespace parse

    namespace encoder {

        /**
         * 解码器函数
         */
        using encoder_fun = std::function<void(const model::h_shared_ptr& h, const model::c_shared_ptr& c,
                                               const model::e_shared_ptr& e, std::vector<char>& buffer)>;
        /**
         * 解码器策略
         */
        extern const std::unordered_map<sl651_2014::model::req_type, encoder_fun> encoder_map;

    } // namespace encoder

} // namespace sl651_2014

#endif // SL_SERVICE_SL651_2014_HPP
