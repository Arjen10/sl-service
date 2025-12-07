//
// Created by Arjen on 2025/3/2.
//


#ifndef SL_REQ_DECODER_HPP
#define SL_REQ_DECODER_HPP

#include <boost/asio/buffer.hpp>
#include <boost/beast.hpp>

#include "byte_buf_reader.hpp"
#include "sl_parser_base.hpp"
#include "sl_full_buf.hpp"
#include "protocol_define.hpp"
#include "basic_decoder.hpp"
#include "protocol/protocol_factory.hpp"

namespace beast = boost::beast;                 // from <boost/beast.hpp>

class sl_req_decoder
{

private:

    decode_state state_;

    std::shared_ptr<sl_basic_decoder> protocol_define_;

    sl_full_buf _full_buf;

public:

    explicit sl_req_decoder();

    std::size_t put(boost::asio::const_buffer buffer, boost::system::error_code &ec);

    sl_full_buf release();

    void reset();

    bool is_done() const;

    std::shared_ptr<sl_basic_decoder> protocol_define();

};


#endif //SL_REQ_DECODER_HPP
