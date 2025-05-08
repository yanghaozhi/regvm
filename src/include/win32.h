#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "getopt.h"

#include "log.h"

#if defined(__clang__)
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#endif


template <typename F> bool map_file(const char* file, F func)
{
	HANDLE fd = CreateFile(
		file,
		GENERIC_READ,          // 只读访问
		FILE_SHARE_READ,       // 允许其他进程读取
		NULL,                  // 默认安全属性
		OPEN_EXISTING,         // 只打开已存在的文件
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, // 普通文件属性，提示系统顺序扫描
		NULL
	);
	if (fd == INVALID_HANDLE_VALUE)
	{
        LOGE("Failed to open file : %s ERROR : %d !!!", file, GetLastError());
		return false;
	}

	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(fd, &fileSize))
	{
        LOGE("Failed to get file size : %s ERROR : %d !!!", file, GetLastError());
		return false;
	}

	HANDLE map = CreateFileMapping(
		fd,
		NULL,                  // 默认安全属性
		PAGE_READONLY,         // 只读保护
		0,                     // 文件映射对象的最大尺寸 (高32位)
		0,                     // 文件映射对象的最大尺寸 (低32位，0表示使用文件大小)
		NULL                   // 映射对象的名称 (可选，通常为 NULL)
	);
	if (map == NULL)
	{
        LOGE("Failed to create file mapping object ERROR : %d !!!", GetLastError());
		return false;
	}

	const void* data = MapViewOfFile(
		map,
		FILE_MAP_READ,         // 只读访问
		0,                     // 文件偏移量 (高32位)
		0,                     // 文件偏移量 (低32位)
		0                      // 要映射的字节数 (0 表示映射从偏移量开始的整个文件)
							   // 或者可以指定 fileSize.QuadPart 来映射特定大小
	);
	if (data == NULL)
	{
        LOGE("Failed to map view of file ERROR : %d !!!", GetLastError());
		return false;
	}


    if (func((const char*)data) == false)
    {
        LOGE("parse file : %s ERROR !!!", file);
        return false;
    }

	UnmapViewOfFile(data);
	CloseHandle(map);
	CloseHandle(fd);

	return true;
}

