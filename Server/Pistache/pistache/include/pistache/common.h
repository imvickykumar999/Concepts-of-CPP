/*
 * SPDX-FileCopyrightText: 2015 Mathieu Stefani
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* common.h
   Mathieu Stefani, 12 August 2015

   A collection of macro / utilities / constants
*/

#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <cstring>

#include <pistache/winornix.h>

#include PST_STRERROR_R_HDR

#include PST_NETDB_HDR
#include PST_SOCKET_HDR

#include <sys/types.h>

#include <pistache/pist_check.h>
#include <pistache/pist_syslog.h>

#define TRY(...)                                               \
    do                                                         \
    {                                                          \
        PST_SOCK_STARTUP_CHECK;                                \
        auto ret = __VA_ARGS__;                                \
        if (ret < 0)                                           \
        {                                                      \
            const char* str = #__VA_ARGS__;                    \
            std::ostringstream oss;                            \
            oss << str << ": ";                                \
            if (errno == 0)                                    \
            {                                                  \
                oss << gai_strerror(static_cast<int>(ret));    \
            }                                                  \
            else                                               \
            {                                                  \
                PST_DECL_SE_ERR_P_EXTRA;                           \
                oss << PST_STRERROR_R_ERRNO; \
            }                                                  \
            PS_LOG_INFO_ARGS("TRY ret %d  errno %d  throw %s", \
                             ret, errno, oss.str().c_str());   \
            PS_LOGDBG_STACK_TRACE;                             \
            oss << " (" << __FILE__ << ":" << __LINE__ << ")"; \
            throw std::runtime_error(oss.str());               \
        }                                                      \
    } while (0)

#define TRY_RET(...)                                           \
    [&]() {                                                    \
        PST_SOCK_STARTUP_CHECK;                                \
        auto ret = __VA_ARGS__;                                \
        if (ret < 0)                                           \
        {                                                      \
            const char* str = #__VA_ARGS__;                    \
            std::ostringstream oss;                            \
            PST_DECL_SE_ERR_P_EXTRA;                               \
            oss << str << ": " << PST_STRERROR_R_ERRNO; \
            PS_LOG_INFO_ARGS("TRY ret %d  errno %d  throw %s", \
                             ret, errno, oss.str().c_str());   \
            PS_LOGDBG_STACK_TRACE;                             \
            oss << " (" << __FILE__ << ":" << __LINE__ << ")"; \
            throw std::runtime_error(oss.str());               \
        }                                                      \
        return ret;                                            \
    }();                                                       \
    (void)0

#define TRY_NULL_RET(...)                                      \
    [&]() {                                                    \
        PST_SOCK_STARTUP_CHECK;                                \
        auto ret = __VA_ARGS__;                                \
        if (ret == nullptr)                                       \
        {                                                      \
            const char* str = #__VA_ARGS__;                    \
            std::ostringstream oss;                            \
            PST_DECL_SE_ERR_P_EXTRA;                               \
            oss << str << ": " << PST_STRERROR_R_ERRNO; \
            PS_LOG_INFO_ARGS("TRY_NULL_RET throw errno %d %s", \
                             errno, oss.str().c_str());        \
            PS_LOGDBG_STACK_TRACE;                             \
            oss << " (" << __FILE__ << ":" << __LINE__ << ")"; \
            throw std::runtime_error(oss.str());               \
        }                                                      \
        return ret;                                            \
    }();                                                       \
    (void)0

struct PrintException
{
    void operator()(std::exception_ptr exc) const
    {
        try
        {
            std::rethrow_exception(exc);
        }
        catch (const std::exception& e)
        {
            std::cerr << "An exception occured: " << e.what() << std::endl;
        }
    }
};
