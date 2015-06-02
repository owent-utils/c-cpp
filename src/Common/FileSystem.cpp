//
// Created by 欧文韬 on 2015/6/2.
//

#include <cstring>
#include <memory>
#include <cstdio>
#include "Common/FileSystem.h"

#ifdef _MSC_VER
#define FUNC_ACCESS(x) _access(x, 0)
#define SAFE_STRTOK_S(...) strtok_s(__VA_ARGS__)
#define FUNC_MKDIR(path, mode) _mkdir(path)

#else

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
        std::string ret;
        ret.resize(MAX_PATH_LEN + 1, 0);

#ifdef _MSC_VER
         _fullpath(const_cast<char*>(ret.data()), dir_path, ret.size());
#else
        readlink(dir_path, const_cast<char*>(ret.data()), ret.size());
#endif

        ret.resize(strlen(ret.c_str()));
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
}
