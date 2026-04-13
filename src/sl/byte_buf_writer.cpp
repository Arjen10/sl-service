//
// Created by Arjen on 2025/1/21.
//

#include "byte_buf_writer.hpp"

#include <boost/algorithm/hex.hpp>

byte_buf_writer::byte_buf_writer(boost::asio::streambuf& buf) : _buf(buf), _os(&_buf) {}

void byte_buf_writer::write_byte(int8_t byte) {
    _os.put(static_cast<char>(byte));
}

void byte_buf_writer::write_byte(std::uint8_t byte) {
    _os.put(static_cast<char>(byte));
}

void byte_buf_writer::write_bytes(const std::vector<int8_t>& bytes) {
    _os.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

void byte_buf_writer::write_bytes(const char* data, std::size_t length) {
    _os.write(data, length);
}

void byte_buf_writer::write_short(int16_t s) {
    _os.write(reinterpret_cast<const char*>(&s), sizeof(s));
}

void byte_buf_writer::write_hex_str(const std::string& hex) {
    std::vector<char> bytes;
    bytes.reserve(hex.length() / 2);
    boost::algorithm::unhex(hex.begin(), hex.end(), std::back_inserter(bytes));
    _os.write(bytes.data(), bytes.size());
}

void byte_buf_writer::write_bcd_byte_from_int64(std::int64_t value) {
    std::string str = std::to_string(value);
    if (str.length() % 2 != 0) {
        str = std::string(str.length() % 2, '0') + str;
    }
    std::vector<char> bytes;
    bytes.reserve(str.length() / 2);
    boost::algorithm::unhex(str.begin(), str.end(), std::back_inserter(bytes));
    for (char b : bytes) {
        _os.put(b);
    }
}

void byte_buf_writer::write_tm(const boost::posix_time::ptime& tm, std::size_t time_length) {
    auto date = tm.date();
    auto time = tm.time_of_day();

    int year = date.year() - 2000;
    write_bcd_byte_from_int64(year);
    write_bcd_byte_from_int64(date.month().as_number());
    write_bcd_byte_from_int64(date.day());
    write_bcd_byte_from_int64(time.hours());
    write_bcd_byte_from_int64(time.minutes());
    if (time_length == 6) {
        write_bcd_byte_from_int64(time.seconds());
    }
}

std::size_t byte_buf_writer::size() const {
    return _buf.size();
}

std::string byte_buf_writer::debug() {
    auto data = _buf.data();
    auto size = boost::asio::buffer_size(data);
    auto* p = static_cast<const char*>(data.data());

    std::ostringstream oss;
    oss << "size: " << size << std::endl;
    oss << "         +-------------------------------------------------+" << std::endl;
    oss << "         |  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f |" << std::endl;
    oss << "+--------+-------------------------------------------------+----------------+" << std::endl;

    for (std::size_t i = 0; i < size; i += 16) {
        oss << std::setfill('0') << std::setw(8) << std::hex << i << "| ";
        for (std::size_t j = 0; j < 16; ++j) {
            if (i + j < size) {
                oss << std::setw(2) << static_cast<int>(static_cast<unsigned char>(p[i + j])) << " ";
            } else {
                oss << "   ";
            }
        }
        oss << " | ";
        for (std::size_t j = 0; j < 16; ++j) {
            if (i + j < size) {
                char ch = p[i + j];
                oss << (ch >= 32 && ch <= 126 ? ch : '.');
            } else {
                oss << " ";
            }
        }
        oss << "   |" << std::endl;
    }
    oss << "+--------+-------------------------------------------------+----------------+" << std::endl;
    return oss.str();
}
