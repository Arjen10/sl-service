//
// Created by Arjen on 2025/3/2.
//

#include "sl_req_decoder.hpp"
#include "protocol/slt427_2021/slt427_2021.hpp"

sl_req_decoder::sl_req_decoder() {
    state_ = decode_state::read_header;
    protocol_define_ = nullptr;
}

std::size_t sl_req_decoder::put(boost::asio::const_buffer buffer, boost::system::error_code& ec) {
    byte_buf_reader reader_helper(buffer);
    std::size_t n = reader_helper.readable_bytes();
    // 准备裁切
    std::vector<int8_t> temp_v;
    switch (state_) {
    case decode_state::read_header: {
        // 这里小于2的原因是，我们最少需要两个字节来确定这个报文的版本信息
        if (n < 2) {
            ec = boost::beast::http::error::need_buffer;
            return 0;
        }
        // 解析定义为空，我们就要从字节中确定协议版本
        if (!protocol_define_) {
            int16_t note;
            reader_helper.get_short(note, reader_helper.read_index());
            protocol_define_ = protocol_factory::create(note);
        }
        // 如果小于了协议头的最大长度，直接返回，让asio继续读
        std::size_t header_max_len = protocol_define_->header_max_len();
        if (n < header_max_len) {
            ec = boost::beast::http::error::need_buffer;
            return 0;
        }
        reader_helper.read_bytes(temp_v, header_max_len);
        this->_full_buf.write_header(temp_v.data(), temp_v.size());
        state_ = decode_state::read_fixed_length_content;
        if (n == header_max_len) {
            ec = boost::beast::http::error::need_buffer;
            return reader_helper.read_index();
        }
    }
    case decode_state::read_fixed_length_content: {
        std::size_t length = protocol_define_->content_length(this->_full_buf.header());
        // 剩余字节是否大于正文长度
        if (reader_helper.readable_bytes() < length) {
            ec = boost::beast::http::error::need_buffer;
            return reader_helper.read_index();
        }
        // 裁切出正文buf
        reader_helper.read_bytes(temp_v, length);
        this->_full_buf.write_content(temp_v.data(), temp_v.size());
        state_ = decode_state::read_end_section;
    }
    case decode_state::read_end_section: {
        auto end_length = protocol_define_->end_length();
        if (reader_helper.readable_bytes() < end_length) {
            ec = boost::beast::http::error::need_buffer;
            return reader_helper.read_index();
        }
        // 裁切出结束部分buf
        reader_helper.read_bytes(temp_v, end_length);
        this->_full_buf.write_end(temp_v.data(), temp_v.size());
        // crc校验
        auto& full_buf = this->_full_buf;
        bool ret = protocol_define_->crc_check(full_buf.header(), full_buf.content(), full_buf.end());
        // 校验不通过
        if (!ret) {
            ec = boost::beast::http::error::bad_value;
            release();
            return reader_helper.read_index();
        }
        state_ = decode_state::done;
    }
    case decode_state::done:
    default:
        break;
    }
    return reader_helper.read_index();
}

sl_full_buf sl_req_decoder::release() {
    sl_full_buf tmp = std::move(this->_full_buf);
    reset();
    return tmp;
}

void sl_req_decoder::reset() {
    this->state_ = decode_state::read_header;
    this->_full_buf.reset();
}

bool sl_req_decoder::is_done() const {
    return this->state_ == decode_state::done;
}

std::shared_ptr<sl_basic_decoder> sl_req_decoder::protocol_define() {
    return this->protocol_define_;
}
