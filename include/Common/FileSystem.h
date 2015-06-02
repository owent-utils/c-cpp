/**
 * @file FileSystem.h
 * @brief 文件系统统一接口
 * Licensed under the MIT licenses.
 *
 * @version 1.0
 * @author OWenT
 * @date 2014.12.15
 *
 * @history
 *
 *
 */

#ifndef _UTIL_COMMON__FILESYSTEM_H
#define _UTIL_COMMON__FILESYSTEM_H

#pragma once

#include <string>
#include <vector>
#include <climits>
#include <cstdio>

#ifdef _MSC_VER
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

namespace util {
    class FileSystem {
    public:
        static const char DIRECTORY_SEPARATOR =
            #ifdef WIN32
                '\\';
            #else
                '/';
            #endif

        static const size_t MAX_PATH_LEN =
#if defined(MAX_PATH)
            MAX_PATH;
#elif defined(_MAX_PATH)
            _MAX_PATH;
#elif defined(PATH_MAX)
            PATH_MAX;
#else
            260;
#endif

    public:
        /**
         * @brief 获取文件内容
         * @param out [OUT] 输出变量
         * @param file_path [IN] 文件路径
         * @param is_binary [IN] 是否是二进制
         * @return 成功返回true
         */
        static bool GetFileContent(std::string& out, const char* file_path, bool is_binary = false);

        /**
         * @brief 获取文件内容
         * @param out [OUT] 输出变量
         * @param path [IN] 路径
         * @param compact [IN] 是否精简路径（这个功能会尽量移除路径中的.和..）
         * @return 成功返回true
         */
        static bool SplitPath(std::vector<std::string>& out, const char* path, bool compact = false);

        /**
         * @brief 检查文件是否存在
         * @param file_path [IN] 文件路径
         * @return 存在且有权限返回true
         */
        static bool IsExist(const char* file_path);

        /**
         * @brief 创建目录
         * @param dir_path [IN] 目录路径
         * @param recursion [IN] 是否递归创建
         * @param mode [IN] 目录权限（Windows下会被忽略）
         * @return 创建成功返回true
         */
        static bool Mkdir(const char* dir_path, bool recursion = false, int mode = 0);

        /**
         * @brief 获取当前运行目录
         * @return 当前运行目录
         */
        static std::string GetCWD();

        /**
         * @brief 获取绝对路径
         * @param dir_path 相对路径
         * @return 当前运行目录
         */
        static std::string GetAbsPath(const char* dir_path);

        /**
         * @brief 移动或重命名文件/目录
         * @param from 原始路径
         * @param to 目标路径
         * @return 成功返回true
         */
        static bool Rename(const char* from, const char* to);

        /**
         * @brief 移除文件/目录
         * @param path 路径
         * @return 成功返回true
         */
        static bool Remove(const char* path);

        /**
         * @brief 打开一个临时文件
         * @return 临时文件
         */
        static FILE* OpenTmpFile();

        /**
         * @brief 生成一个临时文件路径
         * @return 临时文件路径
         */
        static std::string GenerateTmpFilePath();
    };
}


#endif //_UTIL_COMMON__FILESYSTEM_H
