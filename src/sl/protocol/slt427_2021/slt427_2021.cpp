////
//// Created by Arjen on 2024/12/22.
////
//
//
//#include "slt427_2021.hpp"
//
//slt427_2021::device_status::device_status(short status_short) {
//    status = static_cast<unsigned char>((status_short >> 14) & 0x2);  // 保留高 2 位
//    ic = ((status_short >> 13) & 0x1) == 0;  // 检查倒数第 14 位
//    sv = ((status_short >> 12) & 0x1) == 0;  // 检查倒数第 13 位
//    wp = ((status_short >> 11) & 0x1) == 0;  // 检查倒数第 12 位
//    cd = ((status_short >> 10) & 0x1) == 0;  // 检查倒数第 11 位
//    v = static_cast<unsigned char>((status_short >> 9) & 0x1);  // 保留第 10 位
//}
//
//size_t slt427_2021::decoder::header_max_len() {
//    // 当是普通报文时，这里的头部长度最长为3
//    // 当时图片报文时，这里的头部长度最长为11
//    // 两者取最大，就是11
//    return 3;
//}
//
//size_t slt427_2021::decoder::content_length(const boost::asio::const_buffer &buffer) {
//    byte_buf_reader r(buffer);
//    char c;
//    r.get_byte(c, 1);
//    return static_cast<unsigned char>(c);
//}
//
//size_t slt427_2021::decoder::end_length() {
//    return 2;
//}
//
//bool slt427_2021::decoder::version_by_buf(const boost::asio::const_buffer &buffer) {
//    byte_buf_reader r(buffer);
//    char c;
//    r.read_byte(c);
//    return c == static_cast<char>(slt427_2021::frame_symbol::frame_start_end);
//}
//
///**
// * 解析控制域
// * @param content 正文对象
// * @param data 字节指针
// */
//void control_area(std::unique_ptr<slt427_2021::content> &content, byte_buf_reader &reader_helper) {
//    char c;
//    reader_helper.read_byte(c);
//    auto dir = (char) (c > 0 ? 0 : 1);
//    content->dir = dir;
//    auto command_code = (char) (c & 0xf);
//    //拆分标志位，等于1证明分包传输
//    auto div = (char) ((c >> 6) & 1);
//    content->div = div;
//    //拆分计数，BIN码倒计数，等于1时，指针马上前置自增
//    content->divs = 0;
//    if (div == 1) {
//        reader_helper.read_byte(content->divs);
//    }
//    //帧计数位，fcb值等于0表示本次服务传输失败
//    // 无符号位右移4位
//    auto fcb = (unsigned char) ((c >> 4) & 0x3);
//    content->fcb = fcb;
//
//}
//
///**
// * 解析地址域
// * @param content 正文对象
// * @param data 字节指针
// */
//void parse_mpcd(std::unique_ptr<slt427_2021::content> &content, byte_buf_reader &reader_helper) {
//    std::vector<char> stcd_v;
//    // 五字节大小
//    // 有些时候他是 00 ，我们这里无需处理，下面hex自会出手
//    reader_helper.read_bytes(stcd_v, 5);
//    boost::algorithm::hex(stcd_v.begin(), stcd_v.end(), std::back_inserter(content->mpcd));
//    LOG_DEBUG << " mpcd " << content->mpcd;
//}
//
///**
// * 解析afn
// * @param content 正文对象
// * @param data 字节指针
// */
//void parse_afn(std::unique_ptr<slt427_2021::content> &content, byte_buf_reader &reader_helper) {
//    // 兼容用户自定义afn，当afn为0xFF时，afn为用户自定义，我们需要再读一次，共计两字节
//    reader_helper.read_byte(content->afn);
//    if (content->afn == (char) 0xFF) {
//        content->user_afn = true;
//        reader_helper.read_byte(content->afn);
//        return;
//    }
//    content->user_afn = false;
//}
//
///**
// * 格式化时间
// * @param content 正文对象
// * @param reader_helper
// */
//void parse_tm(std::unique_ptr<slt427_2021::content> &content, byte_buf_reader &reader_helper) {
//    LOG_DEBUG << reader_helper.debug();
//    // ss mm HH dd MM yy 格式
//    std::string str_tmp;
//    reader_helper.read_hex_str(str_tmp, 1);
//    int seconds = std::stoi(str_tmp);
//    reader_helper.read_hex_str(str_tmp, 1);
//    int minutes = std::stoi(str_tmp);
//    reader_helper.read_hex_str(str_tmp, 1);
//    int hours = std::stoi(str_tmp);
//    reader_helper.read_hex_str(str_tmp, 1);
//    int day = std::stoi(str_tmp);
//    reader_helper.read_hex_str(str_tmp, 1);
//    int month = std::stoi(str_tmp);
//    reader_helper.read_hex_str(str_tmp, 1);
//    int year = std::stoi(str_tmp);
//    // 创建日期时间对象
//    boost::gregorian::date d(year, month, day);
//    boost::posix_time::time_duration td(hours, minutes, seconds);
//    boost::posix_time::ptime pt(d, td);
//    content->time = boost::posix_time::to_tm(pt);
//    reader_helper.read_byte(content->time_bin);
//}
//
///**
// * 解析正文 / 用户数据区
// * @param content 正文对象
// * @param data 字节指针
// */
//void parse_content(std::unique_ptr<slt427_2021::header> &h,
//                   std::unique_ptr<slt427_2021::content> &content,
//                   sl_full_buf &sl_full_buf) {
//    // 68 1C 68
//    // B3
//    // 53 34 01 07 00
//    // C0
//    // 86 00 00 00 00 83 91 46 19 00 00 00 00 00 33 17 16 08 03 25 00
//    // 47 16
//    byte_buf_reader reader_helper(*sl_full_buf.content());
//    control_area(content, reader_helper);
//    parse_mpcd(content, reader_helper);
//    parse_afn(content, reader_helper);
//    //auto parser = strategy_factory::instance().get_strategy(content->afn);
//    slt427_2021_parse_strategy::mp_q_r parser;
//    parser.parse(h.get(), content.get(), nullptr, reader_helper);
//    short status_short;
//    reader_helper.read_short(status_short);
//    // 处理工况监测字节
//    content->work_state = std::make_shared<work_state>(status_short);
//    reader_helper.read_short(status_short);
//    // 处理设备状态字节
//    content->device_status = std::make_shared<slt427_2021::device_status>(status_short);
//    parse_tm(content, reader_helper);
//}
//
//void parse_header(std::unique_ptr<slt427_2021::header> &h, sl_full_buf &sl_full_buf) {
//    byte_buf_reader reader_helper(*sl_full_buf.content());
//    reader_helper.read_byte(h->frame_start_);
//    reader_helper.read_byte(h->content_length_);
//    reader_helper.read_byte(h->content_start_);
//}
//
///**
// * 收尾工作
// * @param h 头部对象
// * @param content 正文对象
// * @param e 结束对象
// */
//void parse_end(std::unique_ptr<slt427_2021::end> &e, sl_full_buf &sl_full_buf) {
//    byte_buf_reader reader_helper(*sl_full_buf.end());
//    reader_helper.read_byte(e->crc_);
//    reader_helper.read_byte(e->end_byte_);
//}
//
//slt427_2021::decoder::decoder()
//        : state_(decode_state::read_header), header_(std::make_unique<header>()),
//          content_(std::make_unique<content>()), end_(std::make_unique<end>()) {
//
//}
//
//std::string slt427_2021::decoder::protocol_name() {
//    static const std::string protocol_name = "slt427_2021";
//    return protocol_name;
//}
//
//std::any slt427_2021::decoder::crc_calculate(const asio::const_buffer &header, const asio::const_buffer &content,
//                                             const asio::const_buffer &end) {
//    constexpr unsigned int polynomial = 0xe5;  // 多项式
//    constexpr unsigned int initial_remainder = 0;  // 初始值
//    constexpr unsigned int final_xor_value = 0x0;  // 结果异或值
//    constexpr bool reflect_input = false;  // 反转输入
//    constexpr bool reflect_output = false;  // 反转输出
//    using custom_crc_type = boost::crc_optimal<8,
//            polynomial,
//            initial_remainder,
//            final_xor_value,
//            reflect_input,
//            reflect_output>;
//    custom_crc_type crc_calculator;
//    // 用户数据区的crc校验，只校验用户数据区
//    crc_calculator.process_bytes(content.data(), content.size());
//    return static_cast<char>(crc_calculator.checksum());
//}
//
//bool slt427_2021::decoder::crc_check(const boost::asio::const_buffer &header, const boost::asio::const_buffer &content,
//                                     const boost::asio::const_buffer &end) {
//    auto calc_crc = this->crc_calculate(header, content, end);
//    byte_buf_reader end_reader(end);
//    char req_crc;
//    end_reader.read_byte(req_crc);
//    // 从请求中读取crc和计算的crc做比较
//    return req_crc == std::any_cast<char>(calc_crc);
//}
//
//void slt427_2021::decoder::parse(sl_full_buf sl_full_buf) {
//    parse_header(this->header_, sl_full_buf);
//    parse_content(this->header_, this->content_, sl_full_buf);
//    parse_end(this->end_, sl_full_buf);
//}
//
//
//// ----------------------------------------header-----------------------------------------------------------------------
//
//slt427_2021::header::header(char frame_start, unsigned char content_length, char content_start)
//        : frame_start_(frame_start), content_length_(content_length), content_start_(content_start) {
//
//}
//
//char slt427_2021::header::frame_start() const {
//    return this->frame_start_;
//}
//
//unsigned char slt427_2021::header::content_length() const {
//    return this->content_length_;
//}
//
//char slt427_2021::header::content_start() const {
//    return this->content_start_;
//}
//
//slt427_2021::content::content() {
//    this->wr_mp_q_r = nullptr;
//    this->work_state = nullptr;
//    this->time = {};
//}
//
//char slt427_2021::end::crc() const {
//    return crc_;
//}
//
//char slt427_2021::end::end_byte() const {
//    return end_byte_;
//}
