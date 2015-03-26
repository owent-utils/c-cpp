#pragma once

#include <cstdlib>
#include <cstdio>
#include <stdint.h>
#include <string>
#include <inttypes.h>
#include <ctime>
#include <list>
#include "std/functional.h"

#include "DesignPattern/Singleton.h"

#ifndef LOG_WRAPPER_MAX_SIZE_PER_LINE
#define LOG_WRAPPER_MAX_SIZE_PER_LINE 65536
#endif

class LogWrapper : public Singleton<LogWrapper>
{
public:
    struct log_t {
        enum type {
            DEFAULT = 0,   // 服务框架
            DB,            // 数据库服务
            MAX
        };
    };

    struct level_t {
        enum type {
            LOG_LW_DISABLED = 0,     // 关闭日志
            LOG_LW_FATAL,            // 强制输出
            LOG_LW_ERROR,            // 错误
            LOG_LW_WARNING,
            LOG_LW_INFO,
            LOG_LW_NOTICE,
            LOG_LW_DEBUG,
        };
    };

    typedef std::function<void(level_t::type level_id, const char* level, const char* content)> log_handler_t;
    typedef struct {
        level_t::type level_min;
        level_t::type level_max;
        log_handler_t handle;
    } log_router_t;

protected:
    LogWrapper();
    virtual ~LogWrapper();

public:
    // 初始化
    int32_t init(level_t::type level = level_t::LOG_LW_DEBUG);

    void update();

    inline time_t getLogTime() const { return log_time_cache_sec_; }
    inline const tm* getLogTm() const { return log_time_cache_sec_p_; }

    void log(level_t::type level_id, const char* level, const char* file_path, uint32_t line_number, const char* func_name, const char* fnt, ...);

    // 一般日志级别检查
    inline bool check(level_t::type level) {
        return log_level_ >= level;
    }

    inline const std::list<log_router_t>& getLogHandles() const { return log_handlers_; }

    void addLogHandle(log_handler_t h, level_t::type level_min = level_t::LOG_LW_FATAL, level_t::type level_max = level_t::LOG_LW_DEBUG);

    inline void setLevel(level_t::type l) { log_level_ = l; }

    inline level_t::type getLevel() const { return log_level_; }

    inline void setAutoUpdate(bool u) { auto_update_time_ = u; }

    inline bool getAutoUpdate() const { return auto_update_time_; }

    inline bool getEnablePrintFileLocation() const {
        return enable_print_file_location_;
    }

    inline void setEnablePrintFileLocation(bool enable_print_file_location) {
        enable_print_file_location_ = enable_print_file_location;
    }

    inline bool getEnablePrintFunctionName() const {
        return enable_print_function_name_;
    }

    inline void setEnablePrintFunctionName(bool enable_print_function_name) {
        enable_print_function_name_ = enable_print_function_name;
    }

    inline bool getEnablePrintLogType() const {
        return enable_print_log_type_;
    }

    inline void setEnablePrintLogType(bool enable_print_log_type) {
        enable_print_log_type_ = enable_print_log_type;
    }

    inline const std::string& getEnablePrintTime() const {
        return enable_print_time_;
    }

    inline void setEnablePrintTime(const std::string& enable_print_time) {
        enable_print_time_ = enable_print_time;
    }

    // TODO 白名单及用户指定日志输出以后有需要再说

private:
    level_t::type log_level_;
    bool auto_update_time_;
    time_t log_time_cache_sec_;
    tm* log_time_cache_sec_p_;
    std::list<log_router_t> log_handlers_;

    bool enable_print_file_location_;

private:
    bool enable_print_function_name_;
    bool enable_print_log_type_;
    std::string enable_print_time_;
};

#define WDTLOGCHECK(lv)  LogWrapper::Instance()->check(lv)
#define WDTLOGFILENF(name)  LogWrapper::level_t::LOG_LW_##name, #name, __FILE__, __LINE__, __FUNCTION__

// 不同级别日志输出
#define WLOGDEBUG(...) if(WDTLOGCHECK(LogWrapper::level_t::LOG_LW_DEBUG)) { \
    LogWrapper::Instance()->log(WDTLOGFILENF(DEBUG), __VA_ARGS__); \
}

#define WLOGNOTICE(...) if(WDTLOGCHECK(LogWrapper::level_t::LOG_LW_NOTICE)) { \
    LogWrapper::Instance()->log(WDTLOGFILENF(NOTICE), __VA_ARGS__); \
}

#define WLOGINFO(...) if(WDTLOGCHECK(LogWrapper::level_t::LOG_LW_INFO)) { \
    LogWrapper::Instance()->log(WDTLOGFILENF(INFO), __VA_ARGS__); \
}

#define WLOGWARNING(...) if(WDTLOGCHECK(LogWrapper::level_t::LOG_LW_WARNING)) { \
    LogWrapper::Instance()->log(WDTLOGFILENF(WARNING), __VA_ARGS__); \
}

#define WLOGERROR(...) if(WDTLOGCHECK(LogWrapper::level_t::LOG_LW_ERROR)) { \
    LogWrapper::Instance()->log(WDTLOGFILENF(ERROR), __VA_ARGS__); \
}

#define WLOGFATAL(...) if(WDTLOGCHECK(LogWrapper::level_t::LOG_LW_FATAL)) { \
    LogWrapper::Instance()->log(WDTLOGFILENF(FATAL), __VA_ARGS__); \
}

// 控制台输出工具
#ifdef WIN32
#define PSTDTERMCOLOR(code, fmt) fmt
#else
#define PSTDTERMCOLOR(code, fmt) "\033[" #code ";1m" fmt "\033[0m"
#endif

#ifdef _MSC_VER

#define PSTDINFO(fmt, ...)       printf("Info: " fmt, __VA_ARGS__)
#define PSTDNOTICE(fmt, ...)     printf(PSTDTERMCOLOR(36, "Notice: " fmt), __VA_ARGS__)
#define PSTDWARNING(fmt, ...)    printf(PSTDTERMCOLOR(33, "Warning: " fmt), __VA_ARGS__)
#define PSTDERROR(fmt, ...)      printf(PSTDTERMCOLOR(31, "Error: " fmt), __VA_ARGS__)
#define PSTDOK(fmt, ...)         printf(PSTDTERMCOLOR(32, "OK: " fmt), __VA_ARGS__)
//
#ifndef NDEBUG
#define PSTDDEBUG(fmt, ...)     printf(PSTDTERMCOLOR(35, "Debug: " fmt), __VA_ARGS__)
#define PSTDMARK                printf(PSTDTERMCOLOR(35, "Mark: %s:%s (function %s)"), __FILE__, __LINE__, __FUNCTION__)
#else
#define PSTDDEBUG(fmt, ...)
#define PSTDMARK
#endif

#else
#define PSTDINFO(fmt, args...)       printf("Info: " fmt, ##args)
#define PSTDNOTICE(fmt, args...)     printf(PSTDTERMCOLOR(36, "Notice: " fmt), ##args)
#define PSTDWARNING(fmt, args...)    printf(PSTDTERMCOLOR(33, "Warning: " fmt), ##args)
#define PSTDERROR(fmt, args...)      printf(PSTDTERMCOLOR(31, "Error: " fmt), ##args)
#define PSTDOK(fmt, args...)         printf(PSTDTERMCOLOR(32, "OK: " fmt), ##args)
//
#ifndef NDEBUG
#define PSTDDEBUG(fmt, args...)     printf(PSTDTERMCOLOR(35, "Debug: " fmt), ##args)
#define PSTDMARK                    printf(PSTDTERMCOLOR(35, "Mark: %s:%s (function %s)"), __FILE__, __LINE__, __FUNCTION__)
#else
#define PSTDDEBUG(fmt, args...)
#define PSTDMARK
#endif

#endif
