#include "log/LogWrapper.h"

#include "log/LuaLogAdaptor.h"

#ifndef LOG_WRAPPER_DISABLE_LUA_SUPPORT

static int lua_log_adaptor_fn_lua_log(lua_State *L) {
    int top = lua_gettop(L);
    if (top < 2) {
        WLOGERROR("call lua function: lua_log without log level.");
        return 0;
    }

    // TODO log 分类
    // LogWrapperMgr::categorize_t::type cat = static_cast<LogWrapperMgr::categorize_t::type>(luaL_checkinteger(L, 1));

    LogWrapper::level_t::type level = static_cast<LogWrapper::level_t::type>(luaL_checkinteger(L, 2));

    LogWrapper* ptr = LogWrapper::Instance();
    if (ptr && ptr->check(level)) {
        for (int i = 3; i <= top; ++i) {
            const char* content = lua_tostring(L, i);
            if (NULL != content && ptr) {
                ptr->log(level, "Lua", NULL, 0, NULL, "%s",content);
            }
        }
    }

    return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

    int LuaLogAdaptor_openLib(lua_State *L) {
        lua_newtable(L);

        lua_pushinteger(L, static_cast<lua_Integer>(LogWrapper::level_t::LOG_LW_DISABLED));
        lua_setfield(L, -2, "DISABLED");

        lua_pushinteger(L, static_cast<lua_Integer>(LogWrapper::level_t::LOG_LW_FATAL));
        lua_setfield(L, -2, "FATAL");

        lua_pushinteger(L, static_cast<lua_Integer>(LogWrapper::level_t::LOG_LW_ERROR));
        lua_setfield(L, -2, "ERROR");

        lua_pushinteger(L, static_cast<lua_Integer>(LogWrapper::level_t::LOG_LW_WARNING));
        lua_setfield(L, -2, "WARNING");

        lua_pushinteger(L, static_cast<lua_Integer>(LogWrapper::level_t::LOG_LW_INFO));
        lua_setfield(L, -2, "INFO");

        lua_pushinteger(L, static_cast<lua_Integer>(LogWrapper::level_t::LOG_LW_NOTICE));
        lua_setfield(L, -2, "NOTICE"); 

        lua_pushinteger(L, static_cast<lua_Integer>(LogWrapper::level_t::LOG_LW_DEBUG));
        lua_setfield(L, -2, "DEBUG");

        lua_setglobal(L, "lua_log_level_t");

        lua_pushcfunction(L, lua_log_adaptor_fn_lua_log);
        lua_setglobal(L, "lua_log");

        return 0;
    }

#ifdef __cplusplus
}
#endif

#endif
