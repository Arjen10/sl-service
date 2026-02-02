//
// Created by Arjen on 2025/10/18.
//

#ifndef SL_SERVICE_BOOST_THREAD_POOL_HPP
#define SL_SERVICE_BOOST_THREAD_POOL_HPP

#include <boost/asio/thread_pool.hpp>

#include "singleton_template.hpp"

using thread_pool_ptr = std::unique_ptr<boost::asio::thread_pool>;

/**
 * 业务线程池
 */
class sys_thread_pool : public singleton_template<sys_thread_pool> {

  private:
    thread_pool_ptr _thread_pool;
    std::mutex _init_mutex;

  public:

    /**
     * 初始化
     */
    void init(std::uint16_t size);

    /**
     * 获取线程池
     * @return
     */
    thread_pool_ptr& thread_pool();

    ~sys_thread_pool() = default;
};

#endif // SL_SERVICE_BOOST_THREAD_POOL_HPP
