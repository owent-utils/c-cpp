#include "log/LogWrapper.h"

#include "log/LuaLogAdaptor.h"

#ifndef LOG_WRAPPER_DISABLE_LUA_SUPPORT

static int lua_log_adaptor_fn_lua_log(lua_State *L) {
    int top = lua_gettop(L);
    if (top < 1) {
        WLOGERROR("call lua function: lua_log without log level.");
        return 0;
    }

    LogWrapper::level_t::type level = static_cast<LogWrapper::level_t::type>(luaL_checkinteger(L, 1));

    if (WDTLOGCHECK(level)) {
        for (int i = 2; i <= top; ++i) {
            const char* content = lua_tostring(L, i);
            if (NULL != content) {
                LogWrapper::Instance()->log(level, "Lua", NULL, 0, NULL, content);
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
