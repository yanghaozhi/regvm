#pragma once

#include <stdio.h>
#include <string.h>

#define LOG_TRACE   1
#define LOG_DEBUG   2
#define LOG_INFO    3
#define LOG_WARN    4
#define LOG_ERROR   5

#ifdef LOG_LEVEL
#define LOG_ACTIVE  LOG_LEVEL
#else
#define LOG_ACTIVE  0
#endif

#define LOG_IS_ENBALE(level) (LOG_##level >= LOG_ACTIVE)

#ifndef __FILE_NAME__
inline const char* only_file_name(const char* f)
{
    const char* p = strrchr(f, '/');
    return (p != NULL) ? p + 1 : f;
}
#define __FILE_NAME__ only_file_name(__FILE__)
#endif

#ifndef LOG_NO_COLOR
#define FMT_WITH_COLOR(x)   "\e[1;%dm" x "\e[0m "
#define ARG_WITH_COLOR(x)   x
#else
#define FMT_WITH_COLOR(x)   "%s"
#define ARG_WITH_COLOR(x)   ""
#endif

#if defined(_MSC_VER)
#include <winsock2.h>
#include <Windows.h>
#include <format>
#ifndef _DEBUG
#define LOG_COLOR(level, color, fmt, ...) if (LOG_IS_ENBALE(level)) printf(FMT_WITH_COLOR("[%5s] [%s:%d]") fmt " \n", ARG_WITH_COLOR(color), #level, __FILE_NAME__, __LINE__, ##__VA_ARGS__);
#else   //PRINT_TO_CONSOLE
#define LOG_COLOR(level, color, fmt, ...) OutputDebugStringA(std::format("[{}] [{}:{}] " fmt " \n", #level, only_file_name(__FILE__), __LINE__, ##__VA_ARGS__).c_str());
#endif  //PRINT_TO_CONSOLE
#else   //COLOR
#define LOG_COLOR(level, color, fmt, ...) if (LOG_IS_ENBALE(level)) printf(FMT_WITH_COLOR("[%5s] [%s:%d]") fmt " \n", ARG_WITH_COLOR(color), #level, __FILE_NAME__, __LINE__, ##__VA_ARGS__);
#endif  //COLOR

#define LOG_NORMAL  0
#define LOG_RED     31
#define LOG_GREEN   32
#define LOG_YELLOW  33
#define LOG_BLUE    34
#define LOG_PURPLE  35
#define LOG_CYAN    36


#define LOGT(fmt, ...) LOG_COLOR(TRACE, LOG_BLUE, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) LOG_COLOR(DEBUG, LOG_CYAN, fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) LOG_COLOR(INFO,  LOG_GREEN, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) LOG_COLOR(WARN,  LOG_YELLOW, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) LOG_COLOR(ERROR, LOG_RED, fmt, ##__VA_ARGS__)

