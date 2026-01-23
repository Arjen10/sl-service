//
// Created by Arjen on 2025/9/10.
//

#ifndef SL_SERVICE_LOG_HPP
#define SL_SERVICE_LOG_HPP

#include <boost/log/trivial.hpp>

#define LOG_DEBUG                                                                                                      \
    BOOST_LOG_TRIVIAL(debug) << "["                                                                                    \
                             << (strrchr(__FILE__, '/')                                                                \
                                     ? strrchr(__FILE__, '/') + 1                                                      \
                                     : (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))             \
                             << ":" << __LINE__ << "] "

#define LOG_INFO                                                                                                       \
    BOOST_LOG_TRIVIAL(info) << "["                                                                                     \
                            << (strrchr(__FILE__, '/')                                                                 \
                                    ? strrchr(__FILE__, '/') + 1                                                       \
                                    : (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))              \
                            << ":" << __LINE__ << "] "

#define LOG_WARN                                                                                                       \
    BOOST_LOG_TRIVIAL(warning) << "["                                                                                  \
                               << (strrchr(__FILE__, '/')                                                              \
                                       ? strrchr(__FILE__, '/') + 1                                                    \
                                       : (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))           \
                               << ":" << __LINE__ << "] "

#define LOG_ERROR                                                                                                      \
    BOOST_LOG_TRIVIAL(error) << "["                                                                                    \
                             << (strrchr(__FILE__, '/')                                                                \
                                     ? strrchr(__FILE__, '/') + 1                                                      \
                                     : (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))             \
                             << ":" << __LINE__ << "] "

#define LOG_FATAL                                                                                                      \
    BOOST_LOG_TRIVIAL(fatal) << "["                                                                                    \
                             << (strrchr(__FILE__, '/')                                                                \
                                     ? strrchr(__FILE__, '/') + 1                                                      \
                                     : (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))             \
                             << ":" << __LINE__ << "] "

#endif // SL_SERVICE_LOG_HPP
