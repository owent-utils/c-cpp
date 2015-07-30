//
// Created by 欧文韬 on 2015/6/2.
//

#include <cstring>
#include <memory>
#include <cstdio>
#include "Common/FileSystem.h"

#ifdef _MSC_VER
#include <io.h>
#include <direct.h>
#include <Windows.h>
#include "atlconv.h"

#ifdef UNICODE
#define VC_TEXT(x) A2W(x)
#else
#define VC_TEXT(x) x
#endif

#define FUNC_ACCESS(x) _access(x, 0)
#define SAFE_STRTOK_S(...) strtok_s(__VA_ARGS__)
#define FUNC_MKDIR(path, mode) _mkdir(path)

#else

#include <dirent.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/errno.h>

#define FUNC_ACCESS(x) access(x, F_OK)
#define SAFE_STRTOK_S(...) strtok_r(__VA_ARGS__)
#define FUNC_MKDIR(path, mode) mkdir(path, mode)

#endif

namespace util {
    bool FileSystem::GetFileContent(std::string& out, const char* file_path, bool is_binary) {
        FILE* f = NULL;
        if(is_binary) {
            f = fopen(file_path, "rb");
        } else {
            f = fopen(file_path, "r");
        }

        if (NULL == f) {
            return false;
        }

        fseek(f, 0, SEEK_END);
        long len = ftell(f);
        fseek(f, 0, SEEK_SET);

        out.resize(static_cast<size_t>(len));
        fread(const_cast<char*>(out.data()), sizeof(char), static_cast<size_t>(len), f);

        fclose(f);

        return true;
    }

    bool FileSystem::SplitPath(std::vector<std::string>& out, const char* path, bool compact) {
        if (NULL == path) {
            return false;
        }

        char opr_path[MAX_PATH_LEN];
        strncpy(opr_path, path, sizeof(opr_path));

        char* saveptr = NULL;
        char* token = SAFE_STRTOK_S(opr_path, "\\/", &saveptr);
        while (NULL != token) {
            if (0 != strlen(token)) {

                if (compact) {
                    // 紧缩路径
                    if (0 == strcmp("..", token)) {
                        if (!out.empty() && out.back() != "..") {
                            out.pop_back();
                        } else {
                            out.push_back(token);
                        }
                    } else if (0 != strcmp(".", token)) {
                        out.push_back(token);
                    }
                } else {
                    out.push_back(token);
                }
            }
            token = SAFE_STRTOK_S(NULL, "\\/", &saveptr);
        }

        return !out.empty();
    }

    bool FileSystem::IsExist(const char* file_path) {
        return 0 == FUNC_ACCESS(file_path);
    }

    bool FileSystem::Mkdir(const char* dir_path, bool recursion, int mode) {
#ifndef _MSC_VER
        if (0 == mode) {
            mode = S_IRWXU | S_IRWXG | S_IRGRP | S_IWGRP | S_IROTH;
        }
#endif
        if (!recursion) {
            return 0 == FUNC_MKDIR(dir_path, mode);
        }

        std::vector<std::string> path_segs;
        SplitPath(path_segs, dir_path, true);

        if (path_segs.empty()) {
            return false;
        }

        std::string now_path;
        // 留一个\0和一个分隔符位
        now_path.reserve(strlen(dir_path) + 2);

        for(size_t i = 0; i < path_segs.size(); ++ i) {
            now_path += path_segs[i];

            if (false == IsExist(now_path.c_str())) {
                if (false == Mkdir(now_path.c_str(), false, mode)) {
                    return false;
                }
            }

            now_path += DIRECTORY_SEPARATOR;
        }

        return true;
    }

    std::string FileSystem::GetCWD() {
        std::string ret;
#ifdef _MSC_VER
        ret = _getcwd( NULL, 0 );
#else
        ret = getcwd( NULL, 0 );
#endif

        return ret;
    }

    std::string FileSystem::GetAbsPath(const char* dir_path) {
        if (IsAbsPath(dir_path)) {
            return dir_path;
        }

        std::string ret;
        ret.reserve(MAX_PATH_LEN);

        std::vector<std::string> out;

        std::string cwd = GetCWD();
        SplitPath(out, (cwd + DIRECTORY_SEPARATOR + dir_path).c_str(), true);

        if ('\\' == cwd[0] || '/' == cwd[0] ) {
            ret += DIRECTORY_SEPARATOR;
        }

        if (!out.empty()) {
            ret += out[0];
        }

        for (size_t i = 1; i < out.size(); ++ i) {
            ret += DIRECTORY_SEPARATOR;
            ret += out[i];
        }

        return ret;
    }

    bool FileSystem::Rename(const char* from, const char* to) {
        return 0 == rename(from, to);
    }

    bool FileSystem::Remove(const char* path) {
        return 0 == remove(path);
    }

    FILE* FileSystem::OpenTmpFile() {
        return tmpfile();
    }

    std::string FileSystem::GenerateTmpFilePath() {
        return tmpnam(NULL);
    }

    int FileSystem::ScanDir(const char* dir_path, std::list<std::string>& out, int options) {
        int ret = 0;
        std::string base_dir = dir_path? dir_path: "";

        // 转为绝对路径
        if ((options & dir_opt_t::EN_DOT_ABSP) && false == IsAbsPath(base_dir.c_str())) {
            if (base_dir.empty()) {
                base_dir = GetCWD();
            } else {
                base_dir = GetAbsPath(base_dir.c_str());
            }
        }

#ifdef _MSC_VER

        if (!base_dir.empty()) {
            base_dir += DIRECTORY_SEPARATOR;
        }

        _finddata_t child_node;
        intptr_t cache = _findfirst((base_dir + "*").c_str(), &child_node);

        if (-1 == cache) {
            return errno;
        }

        do {

            std::string child_path;
            child_path.reserve(MAX_PATH_LEN);
            child_path = base_dir;
            child_path += child_node.name;

            int accept = 0;
            bool is_link = false;

            // Windows 版本暂不支持软链接
            if (_A_SUBDIR & child_node.attrib) {
                accept = options & dir_opt_t::EN_DOT_TDIR;
            } else if (_A_NORMAL & child_node.attrib) {
                accept = options & dir_opt_t::EN_DOT_TREG;
            } else {
                accept = options & dir_opt_t::EN_DOT_TOTH;
            }

            USES_CONVERSION;

            DWORD flag = GetFileAttributes(VC_TEXT(child_path.c_str()));
            if (FILE_ATTRIBUTE_REPARSE_POINT & flag) {
                accept = options & dir_opt_t::EN_DOT_TLNK;
                is_link = true;
            }

            // 类型不符合则跳过
            if (0 == accept) {
                continue;
            }

            // 是否排除 . 和 ..
            if (0 == strcmp(".", child_node.name) || 0 == strcmp("..", child_node.name)) {
                if (!(options & dir_opt_t::EN_DOT_SELF)) {
                    continue;
                }
            } else {

                // 递归扫描（软链接不扫描，防止死循环）
                if (!is_link && (_A_SUBDIR & child_node.attrib) && (options & dir_opt_t::EN_DOT_RECU)) {
                    ScanDir(child_path.c_str(), out, options & (~dir_opt_t::EN_DOT_SELF));
                    continue;
                }

                // 解析软链接
                if ((FILE_ATTRIBUTE_REPARSE_POINT & flag) && (options & dir_opt_t::EN_DOT_RLNK)) {
                    child_path = GetAbsPath(child_path.c_str());
                }
            }

            // 普通追加目录
            out.push_back(child_path);
        } while((ret = _findnext( cache, &child_node )) == 0);

        _findclose(cache);

        if (ENOENT == ret) {
            return 0;
        }
#else
        DIR* dir = NULL;
        if (base_dir.empty()) {
            dir = opendir(".");
        } else {
            dir = opendir(base_dir.c_str());
        }
        if (NULL == dir) {
            return errno;
        }

        struct dirent child_node;
        struct dirent* cache = NULL;

        do {
            child_node.d_name[0] = '\0';
            ret = readdir_r(dir, &child_node, &cache);
            if(ret < 0) {
                break;
            }

            // reach the end
            if (NULL == cache) {
                break;
            }

            int accept = 0;
            switch (child_node.d_type) {
                case DT_DIR: {
                    accept = options & dir_opt_t::EN_DOT_TDIR;
                    break;
                }

                case DT_REG: {
                    accept = options & dir_opt_t::EN_DOT_TREG;
                    break;
                }

                case DT_LNK: {
                    accept = options & dir_opt_t::EN_DOT_TLNK;
                    break;
                }

                case DT_SOCK: {
                    accept = options & dir_opt_t::EN_DOT_TSOCK;
                    break;
                }

                default: {
                    accept = options & dir_opt_t::EN_DOT_TOTH;
                    break;
                }
            }

            // 类型不符合则跳过
            if (0 == accept) {
                continue;
            }

            std::string child_path;
            child_path.reserve(MAX_PATH_LEN);
            if (!base_dir.empty()) {
                child_path += base_dir + DIRECTORY_SEPARATOR;
            }
            child_path += child_node.d_name;

            // 是否排除 . 和 ..
            if (0 == strcmp(".", child_node.d_name) || 0 == strcmp("..", child_node.d_name)) {
                if (!(options & dir_opt_t::EN_DOT_SELF)) {
                    continue;
                }
            } else {
                // 递归扫描（软链接不扫描，防止死循环）
                if (DT_DIR == child_node.d_type && (options & dir_opt_t::EN_DOT_RECU)) {
                    ScanDir(child_path.c_str(), out, options & (~dir_opt_t::EN_DOT_SELF));
                    continue;
                }

                // 解析软链接
                if (DT_LNK == child_node.d_type && (options & dir_opt_t::EN_DOT_RLNK)) {
                    child_path = GetAbsPath(child_path.c_str());
                }
            }

            // 普通追加目录
            out.push_back(child_path);
        } while(true);

        closedir(dir);

#endif

        return ret;
    }


    bool FileSystem::IsAbsPath(const char* dir_path) {
        if (NULL == dir_path) {
            return false;
        }

        if(dir_path[0] == '/') {
            return true;
        }

#ifdef WIN32
        if (((dir_path[0] >= 'a' && dir_path[0] <= 'z') || (dir_path[0] >= 'A' && dir_path[0] <= 'Z')) && dir_path[1] == ':' ) {
            return true;
        }
#endif

        return false;
    }
}
