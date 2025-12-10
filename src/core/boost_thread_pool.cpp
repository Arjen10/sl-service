//
// Created by Arjen on 2025/10/18.
//

#include "boost_thread_pool.hpp"

boost::asio::thread_pool& thread_pool()
{
    //todo 后续处理
    static boost::asio::thread_pool tp(4);
    return tp;
}
