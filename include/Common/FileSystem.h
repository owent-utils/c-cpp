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
#include <list>
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

        struct dir_opt_t {
            enum type {
                EN_DOT_ABSP     = 0x0001,       // 转换为绝对路径
                EN_DOT_SELF     = 0x0002,       // 包含.和..
                EN_DOT_RLNK     = 0x0004,       // 解析符号链接
                EN_DOT_RECU     = 0x0010,       // 对目录递归扫描而不是列举出出目录名

                EN_DOT_TDIR     = 0x0100,       // 包含目录
                EN_DOT_TREG     = 0x0200,       // 包含文件
                EN_DOT_TLNK     = 0x0400,       // 包含符号链接
                EN_DOT_TSOCK    = 0x0800,       // 包含Unix Sock
                EN_DOT_TOTH     = 0x1000,       // 其他类型

                EN_DOT_DAFAULT  = 0xFF00,       // 默认规则
            };
        };
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

        /**
         * @brief 列举目录下所有文件
         * @param dir_path 目录路径
         * @param out 录下所有文件路径
         * @return 成功返回0，错误返回错误码(不同平台错误码不同)
         */
        static int ScanDir(const char* dir_path, std::list<std::string>& out, int options = dir_opt_t::EN_DOT_DAFAULT);

        /**
         * @brief 判断是否是绝对路径
         * @param dir_path 目录路径
         * @return 是绝对路径返回true
         */
        static bool IsAbsPath(const char* dir_path);
    };
}


#endif //_UTIL_COMMON__FILESYSTEM_H
