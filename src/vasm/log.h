#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>


#ifndef SPDLOG_H

#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER
#define only_file_name(x) x
#else
#ifdef __FILE_NAME__
#define only_file_name(x)   __FILE_NAME__
#else
constexpr const char* only_file_name(const char* f)
{
    const char* p = strrchr(f, '/');
    return (p != NULL) ? p + 1 : f;
}
#endif
#endif  //only_file_name

#if defined(_MSC_VER)
#include <winsock2.h>
#include <Windows.h>
#include <format>
#ifndef _DEBUG
#define COLOR(level, color, fmt, ...) printf("%s", std::format("\033[1;{}m[{}] [{}:{}]\033[0m " fmt " \n", color, #level, only_file_name(__FILE__), __LINE__, ##__VA_ARGS__).c_str());
#else   //PRINT_TO_CONSOLE
#define COLOR(level, color, fmt, ...) OutputDebugStringA(std::format("[{}] [{}:{}] " fmt " \n", #level, only_file_name(__FILE__), __LINE__, ##__VA_ARGS__).c_str());
#endif  //PRINT_TO_CONSOLE
#else   //COLOR
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <fmt/printf.h>
#include <fmt/format.h>
#define COLOR(level, color, format, ...) fmt::print("\e[1;{}m[{}] [{}:{}]\e[0m " format " \n", color, #level, only_file_name(__FILE__), __LINE__, ##__VA_ARGS__);
#endif  //COLOR

#define NORMAL  0
#define RED     31
#define GREEN   32
#define YELLOW  33
#define BLUE    34
#define PURPLE  35
#define CYAN    36


#define TRACE(fmt, ...) COLOR(TRACE, PURPLE, fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...) COLOR(DEBUG, BLUE, fmt, ##__VA_ARGS__)
#define INFO(fmt, ...)  COLOR(INFO,  GREEN, fmt, ##__VA_ARGS__)
#define WARN(fmt, ...)  COLOR(WARN,  YELLOW, fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) COLOR(ERROR, RED, fmt, ##__VA_ARGS__)

#else   //SPDLOG_H

#include <fmt/chrono.h>
#include <fmt/printf.h>


#define TRACE SPDLOG_TRACE
#define DEBUG SPDLOG_DEBUG
#define INFO  SPDLOG_INFO
#define WARN  SPDLOG_WARN
#define ERROR SPDLOG_ERROR
#define FATAL SPDLOG_CRITICAL


namespace logging
{
    enum level
    {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
    };

    inline void set_level(int level)
    {
        spdlog::set_pattern("%^[%L] [%H:%M:%S.%e] [%t] [%s:%#] [%i]%$ %v");

        switch (level)
        {
#define SET_LEVEL(k, v)                             \
        case k :                                    \
            spdlog::set_level(spdlog::level::v);    \
            break;
            SET_LEVEL(TRACE, trace)
            SET_LEVEL(DEBUG, debug)
            SET_LEVEL(INFO,  info)
            SET_LEVEL(WARN,  warn)
            SET_LEVEL(ERROR, err)
            SET_LEVEL(FATAL, critical)
        default:
            spdlog::set_level(spdlog::level::trace);
#undef SET_LEVEL
        }
    }
    inline void set_level(const char* level)
    {
        switch (*(uint32_t*)level)
        {
#define SET_LEVEL(v, k)                             \
        case k:                                     \
            set_level(logging::v);                  \
            break;
            SET_LEVEL(TRACE, 0x63617274)
            SET_LEVEL(DEBUG, 0x75626564)
            SET_LEVEL(INFO,  0x6f666e69)
            SET_LEVEL(WARN,  0x6e726177)
            SET_LEVEL(ERROR, 0x6f727265)
            SET_LEVEL(FATAL, 0x61746166)
        default:
            spdlog::set_level(spdlog::level::trace);
#undef SET_LEVEL
        }
    }
};

#endif  //SPDLOG_H
