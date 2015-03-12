/**
 * @file StringCommon.h
 * @brief 
 * Licensed under the MIT licenses.
 *
 */

#ifndef _UTIL_URI_STRINGCOMMON_H_
#define _UTIL_URI_STRINGCOMMON_H_

#include <cstring>

#pragma once

#ifdef _MSC_VER
#define STRING_NCASE_CMP(l, r) _stricmp(l, r)

#else
#define STRING_NCASE_CMP(l, r) strcasecmp(l, r)

#endif


#endif
