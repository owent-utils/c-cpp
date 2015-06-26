# 默认配置选项
#####################################################################
option(BUILD_SHARED_LIBS "Build shared libraries (DLLs)." OFF)
set(ENABLE_MIXEDINT_MAGIC_MASK 0 CACHE STRING "Integer mixed magic mask")

if(NOT ENABLE_LUA_SUPPORT)
    option(ENABLE_LUA_SUPPORT "Force open lua support." OFF)
endif()
