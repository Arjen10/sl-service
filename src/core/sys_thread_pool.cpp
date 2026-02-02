//
// Created by Arjen on 2025/10/18.
//

#include "sys_thread_pool.hpp"

#include "log.hpp"

void sys_thread_pool::init(std::uint16_t size) {
    if (size < 1) {
        LOG_FATAL << " 输入的业务线程数量："<< size <<" 小于 1";
        std::exit(EXIT_FAILURE);
    }
    std::lock_guard<std::mutex> lock_guard(this->_init_mutex);
    if (!_thread_pool) {
        _thread_pool = std::make_unique<boost::asio::thread_pool>(size);
    }
}

thread_pool_ptr& sys_thread_pool::thread_pool() {
    return this->_thread_pool;
}
