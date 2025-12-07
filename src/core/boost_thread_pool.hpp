//
// Created by Arjen on 2025/10/18.
//

#ifndef SL_SERVICE_BOOST_THREAD_POOL_HPP
#define SL_SERVICE_BOOST_THREAD_POOL_HPP

#include <boost/asio/thread_pool.hpp>

/**
 * 获取线程池
 * @return
 */
boost::asio::thread_pool& thread_pool();

#endif //SL_SERVICE_BOOST_THREAD_POOL_HPP
