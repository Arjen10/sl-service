//
// Created by Arjen on 2025/1/21.
//

#include "byte_buf_reader.hpp"

#include <stdexcept>

byte_buf_reader::byte_buf_reader(const boost::asio::const_buffer& buffer) : buffer_(buffer), read_index_(0) {
    p_ = static_cast<char const*>(buffer_.data());
}

void byte_buf_reader::read_byte(int8_t& byte) {
    byte = read_byte();
}

int8_t byte_buf_reader::read_byte() {
    if (readable_bytes() < 1) {
        throw std::out_of_range("read_byte out of range");
    }
    return p_[read_index_++];
}

void byte_buf_reader::get_byte(int8_t& byte, std::size_t index) {
    byte = p_[index];
}

void byte_buf_reader::get_bytes(std::vector<int8_t>& bytes, std::size_t start, std::size_t length) {
    bytes.clear();
    for (std::size_t i = start; i < length; ++i) {
        bytes.push_back(p_[i]);
    }
}

void byte_buf_reader::read_byte(std::uint8_t& byte) {
    if (readable_bytes() < 1) {
        throw std::out_of_range("read_byte out of range");
    }
    byte = p_[read_index_++];
}

void byte_buf_reader::read_bytes(std::vector<int8_t>& bytes, std::size_t length) {
    if (readable_bytes() < length) {
        throw std::out_of_range("read_bytes out of range");
    }
    bytes.clear();
    for (std::size_t i = 0; i < length; ++i) {
        bytes.push_back(p_[read_index_ + i]);
    }
    read_index_ += length;
}

void byte_buf_reader::read_short(int16_t& s) {
    if (readable_bytes() < 2) {
        throw std::out_of_range("read_short out of range");
    }
    get_short(s, read_index());
    // 读指针加 2 即可
    read_index_ += 2;
}

void byte_buf_reader::get_short(int16_t& s, std::size_t index) {
    const auto size = boost::asio::buffer_size(buffer_);
    if (index >= size || size - index < 2) {
        throw std::out_of_range("get_short out of range");
    }
    const auto high = static_cast<unsigned char>(p_[index]);
    const auto low = static_cast<unsigned char>(p_[index + 1]);
    s = static_cast<int16_t>((high << 8) | low);
}

void byte_buf_reader::read_hex_str(std::string& str, std::size_t length) {
    get_hex_str(str, read_index_, length);
    read_index_ += length;
}

void byte_buf_reader::get_hex_str(std::string& str, std::size_t length) {
    this->get_hex_str(str, read_index(), length);
}

void byte_buf_reader::get_hex_str(std::string& str, std::size_t start_index, std::size_t length) {
    // 循环读取每个字节并将其转化为十六进制字符串
    std::ostringstream oss;
    for (std::size_t i = 0; i < length; ++i) {
        unsigned char byte = p_[start_index + i]; // 获取当前字节
        // 转换为两位十六进制字符串
        oss << std::setw(2) << std::setfill('0') << std::hex << (int)byte;
    }
    // 将该十六进制字符串添加到结果字符串中
    str.append(oss.str());
}

std::size_t byte_buf_reader::read_index() const {
    return read_index_;
}

std::size_t byte_buf_reader::readable_bytes() const {
    return buffer_.size() - read_index_;
}

void byte_buf_reader::skip(std::size_t length) {
    if (this->readable_bytes() < length) {
        std::stringstream ss;
        ss << "跳过的字节数: " << length << " 大于了可读字节数: " << this->readable_bytes();
        throw std::runtime_error(ss.str());
    }
    read_index_ += length;
}

std::int64_t byte_buf_reader::read_bcd_byte_to_int64(std::size_t length) {
    std::int64_t ret = 0;
    for (std::size_t i = 0; i < length; ++i) {
        auto b = static_cast<unsigned char>(p_[read_index_]);
        // 每个字节贡献两位
        ret = ret * 100 + ((b >> 4) & 0xF) * 10 + (b & 0xF);
        read_index_++;
    }
    return ret;
}

void byte_buf_reader::read_tm(boost::posix_time::ptime& tm, std::size_t time_length) {
    // 2000 + year
    int year = 2000 + read_bcd_byte_to_int64(1);
    int month = read_bcd_byte_to_int64(1);
    int day = read_bcd_byte_to_int64(1);
    int hour = read_bcd_byte_to_int64(1);
    int min = read_bcd_byte_to_int64(1);
    int sec = (time_length == 6) ? read_bcd_byte_to_int64(1) : 0;
    tm = boost::posix_time::ptime(boost::gregorian::date(year, month, day), boost::posix_time::hours(hour) +
                                                                                boost::posix_time::minutes(min) +
                                                                                boost::posix_time::seconds(sec));
}

std::string byte_buf_reader::debug() {
    size_t size = boost::asio::buffer_size(buffer_);
    std::ostringstream oss; // 使用字符串流代替直接输出
    oss << "read index: " << read_index() << " readable_bytes: " << readable_bytes() << std::endl;
    oss << "         +-------------------------------------------------+" << std::endl;
    oss << "         |  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f |" << std::endl;
    oss << "+--------+-------------------------------------------------+----------------+" << std::endl;
    // 从 read_index() 开始遍历
    size_t start_index = read_index(); // 获取读取的起始位置
    for (size_t i = start_index; i < size; i += 16) {
        oss << std::setfill('0') << std::setw(8) << std::hex << i << "| ";
        // 打印16个字节的16进制，确保每个字节之间有空格，且对齐
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < size) {
                oss << std::setw(2) << static_cast<int>(static_cast<unsigned char>(p_[i + j])) << " ";
            } else {
                oss << "   "; // 如果当前没有数据，填充空格
            }
        }
        oss << " | ";
        // 打印对应的ASCII字符，非可打印字符替换为 '.'
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < size) {
                char ch = p_[i + j];
                oss << (ch >= 32 && ch <= 126 ? ch : '.');
            } else {
                oss << " "; // 如果当前没有数据，填充空格
            }
        }
        oss << "   |" << std::endl;
    }
    oss << "+--------+-------------------------------------------------+----------------+" << std::endl;
    return oss.str(); // 将结果赋值到 str
}
