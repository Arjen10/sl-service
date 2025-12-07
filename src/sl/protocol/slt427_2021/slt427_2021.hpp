////
//// Created by Arjen on 2024/12/22.
////
//
//#ifndef SLT427_2021_HPP
//#define SLT427_2021_HPP
//
//#include <boost/algorithm/hex.hpp>
//#include <boost/log/trivial.hpp>
//#include <boost/crc.hpp>
//#include <boost/date_time.hpp>
//#include <iostream>
//
//#include "../../basic_decoder.hpp"
//#include "../../../byte_helper.hpp"
//#include "parse_strategy.hpp"
//#include "../../../gb/szy302_2017.hpp"
//#include "../../protocol_define.hpp"
//
//class wr_mp_q_r;
//
//class work_state;
//
//namespace slt427_2021 {
//
//    /**
//    * 时间标识符，长度7个字节
//    */
//    const char TIME_LENGTH = 7;
//
//    /**
//     * RTU设备状态字节数
//     */
//    const char RTU_STATE = 4;
//
//    /**
//     * 结尾长度（共计两字节） = crc（一字节）+ 结束符（一字节）
//     */
//    const char END_LENGTH = 2;
//
//    class decoder;
//
//    enum class function_enum : char {
//
//        connect_monitor = 0x02,
//
//        connect_monitor_sign_in = static_cast<char>(0xF0),
//
//        connect_monitor_sign_out = static_cast<char>(0xF1),
//
//        connect_monitor_keep_online = static_cast<char>(0xF2),
//
//        auto_real_time_data = static_cast<char>(0xC0),
//
//        select_clock = 0x51,
//
//        select_picture = 0x61,
//
//        auto_picture = static_cast<char>(0x83),
//
//        auto_v = static_cast<char>(0x84),
//
//        real_time_value = static_cast<char>(0xB0)
//    };
//
//    enum class frame_symbol : char {
//
//        /**
//         * 帧起始和报文正文起始符
//         */
//        frame_start_end = 0x68,
//
//        /**
//         * 帧结束符
//         */
//        frame_end = 0x16
//
//    };
//
//    class device_status {
//    public:
//
//        device_status(short status_short);
//
//        /**
//         * 设备状态：
//         *     0：终端机在自报、遥测工作状态
//         *     1：终端机在自报确认工作状态
//         *     2：终端机在遥测工作状态
//         *     3：终端机在调试或者维修状态
//         */
//        unsigned char status;
//
//        /**
//         * 终端机IC卡功能是否有效
//         */
//        bool ic;
//
//        /**
//         * 定值控制是否投入
//         */
//        bool sv;
//
//        /**
//         * 水泵工作状态
//         */
//        bool wp;
//
//        /**
//         * 终端机箱门状态，true：开启、关闭
//         */
//        bool cd;
//
//        /**
//         * 电源工作状态，0：AC220V供电，1蓄电池供电
//         */
//        char v;
//
//    };
//
//    class header {
//
//        friend decoder;
//
//    public:
//
//        explicit header() = default;
//
//        header(char frame_start, unsigned char content_lent, char content_start);
//
//        // 帧起始和报文正文起始符
//        char frame_start_;
//
//        // 报文正文长度
//        unsigned char content_length_;
//
//        // 报文正文起始符
//        char content_start_;
//
//        std::string header_hex;
//
//    public:
//
//        char frame_start() const;
//
//        unsigned char content_length() const;
//
//        char content_start() const;
//    };
//
//    class content {
//
//    public:
//
//        content();
//
//    public:
//        /**
//         * 控制域字节
//         */
//        char c;
//
//        /**
//         * 传输方向位，0是下行，1是上行
//         */
//        char dir;
//
//        /**
//         * 拆分标志位，等于1证明分包传输
//         */
//        char div;
//
//        /**
//         * 拆分计数，BIN码倒计数
//         */
//        char divs;
//
//        /**
//         * 帧计数位，fcb值等于0表示本次服务传输失败
//         */
//        unsigned char fcb;
//
//        /**
//         * 地址域
//         */
//        std::string mpcd;
//
//        /**
//         * 是否是用户自定义afn
//         */
//        bool user_afn;
//
//        /**
//         * afn
//         */
//        char afn;
//
//        /**
//         * 时间
//         */
//        std::tm time;
//
//        /**
//         * 传输延迟，单位分钟
//         */
//        char time_bin;
//
//        /**
//         * 流量、水量信息
//         */
//        std::shared_ptr<wr_mp_q_r> wr_mp_q_r;
//
//        /**
//         * 工况
//         */
//        std::shared_ptr<work_state> work_state;
//
//        /**
//         * 设备状态
//         */
//        std::shared_ptr<device_status> device_status;
//
//        std::vector<char> buf_;
//
//    };
//
//    class end {
//
//    public:
//        /**
//         * crc校验符
//         */
//        char crc_;
//
//        /**
//         * 结束字节
//         */
//        char end_byte_;
//
//    public:
//
//        char crc() const;
//
//        char end_byte() const;
//
//    };
//
//    class decoder : public sl_basic_decoder {
//
//    private:
//
//        decode_state state_;
//
//        std::unique_ptr<header> header_;
//
//        std::unique_ptr<content> content_;
//
//        std::unique_ptr<end> end_;
//
//    public:
//
//        explicit decoder();
//
//        ~decoder() = default;
//
//        std::string protocol_name() override;
//
//        void parse(sl_full_buf sl_full_buf) override;
//
//        //sl_full_buf encod() override;
//
//        /**
//         * slt427-2021
//         * polynomial = x7+ x6+ x5+ x2+ x0
//         */
//        bool crc_check(const boost::asio::const_buffer &header,
//                       const boost::asio::const_buffer &content,
//                       const boost::asio::const_buffer &end) override;
//
//        std::any crc_calculate(const asio::const_buffer &header,
//                               const asio::const_buffer &content,
//                               const asio::const_buffer &end) override;
//
//        std::size_t header_max_len() override;
//
//        std::size_t end_length() override;
//
//        std::size_t content_length(const boost::asio::const_buffer &buffer) override;
//
//        bool version_by_buf(const boost::asio::const_buffer &buffer) override;
//
//    };
//
//}
//
//#endif //SLT427_2021_HPP
