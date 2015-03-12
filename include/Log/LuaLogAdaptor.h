#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lauxlib.h"


int LuaLogAdaptor_openLib(lua_State *L);

#ifdef __cplusplus
}
#endif
