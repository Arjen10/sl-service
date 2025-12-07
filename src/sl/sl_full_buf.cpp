//
// Created by Arjen on 2025/3/2.
//

#include "sl_full_buf.hpp"

sl_full_buf::sl_full_buf() {
    this->_header = std::make_unique<asio::streambuf>();
    this->_content = std::make_unique<asio::streambuf>();
    this->_end = std::make_unique<asio::streambuf>();
}

void sl_full_buf::write_header(const int8_t *data, size_t size) {
    this->write(data, size, this->_header);
}

void sl_full_buf::write_content(const int8_t *data, size_t size) {
    this->write(data, size, this->_content);
}

void sl_full_buf::write_end(const int8_t *data, size_t size) {
    this->write(data, size, this->_end);
}

asio::const_buffer sl_full_buf::header() const {
    return this->_header->data();
}

asio::const_buffer sl_full_buf::content() const {
    return this->_content->data();
}

asio::const_buffer sl_full_buf::end() const {
    return this->_end->data();
}

void sl_full_buf::write(int8_t byte, std::unique_ptr<asio::streambuf> &buf) {
    std::ostream os(buf.get());
    os.put(static_cast<char>(byte));
}

void sl_full_buf::write(const int8_t *data, size_t size, std::unique_ptr<asio::streambuf> &buf) {
    std::ostream os(buf.get());
    os.write(reinterpret_cast<const char *>(data), size);
}

void sl_full_buf::reset()
{
    this->_header = std::make_unique<asio::streambuf>();
    this->_content = std::make_unique<asio::streambuf>();
    this->_end = std::make_unique<asio::streambuf>();
}

sl_full_buf::sl_full_buf(sl_full_buf &&other) noexcept
        : _header(std::move(other._header)),
          _content(std::move(other._content)),
          _end(std::move(other._end))
{
}
