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


namespace util {
    namespace string {
        /**
         * @brief 字符串转整数
         * @param out 输出的整数
         * @param str 被转换的字符串
         * @note 性能肯定比sscanf系，和iostream系高。strtol系就不知道了
         */
        template<typename T>
        void str2int(T& out, const char* str) {
            out = static_cast<T>(0);
            if (NULL == str || !(*str)) {
                return;
            }

            if ('0' == str[0] && 'x' == str[1]) { // hex
                for (size_t i = 2; str[i]; ++i) {
                    char c = static_cast<char>(::tolower(str[i]));
                    if (c >= '0' && c <= '9') {
                        out <<= 4;
                        out += c - '0';
                    } else if (c >= 'a' && c <= 'f') {
                        out <<= 4;
                        out += c - 'a' + 10;
                    } else {
                        break;
                    }
                }
            } else if ('\\' == str[0]) { // oct
                for (size_t i = 0; str[i] >= '0' && str[i] < '8'; ++i) {
                    out <<= 3;
                    out += str[i] - '0';
                }
            } else { // dec
                for (size_t i = 0; str[i] >= '0' && str[i] <= '9'; ++i) {
                    out *= 10;
                    out += str[i] - '0';
                }
            }
        }

        /**
         * @brief 字符转十六进制表示
         * @param out 输出的字符串(缓冲区长度至少为2)
         * @param c 被转换的字符
         * @param upper_case 输出大写字符？
         */
        template<typename TStr, typename TCh>
        void hex(TStr* out, TCh c, bool upper_case = false) {
            out[0] = static_cast<TStr>((c >> 4) & 0x0F);
            out[1] = static_cast<TStr>(c & 0x0F);

            for (int i = 0; i < 2; ++ i) {
                if (out[i] > 9) {
                    out[i] += (upper_case ? 'A' : 'a') - 10;
                } else {
                    out[i] += '0';
                }
            }
        }

        /**
         * @brief 字符转8进制表示
         * @param out 输出的字符串(缓冲区长度至少为3)
         * @param c 被转换的字符
         * @param upper_case 输出大写字符？
         */
        template<typename TStr, typename TCh>
        void oct(TStr* out, TCh c) {
            out[0] = static_cast<TStr>(((c >> 6) & 0x07) + '0');
            out[1] = static_cast<TStr>(((c >> 3) & 0x07) + '0');
            out[2] = static_cast<TStr>((c & 0x07) + '0');
        }

        /**
         * @brief 字符转8进制表示
         * @param src 输入的buffer
         * @param ss 输入的buffer长度
         * @param out 输出buffer
         * @param os 输出buffer长度，回传输出缓冲区使用的长度
         */
        template<typename TCh>
        void serialization(const void* src, size_t ss, TCh* out, size_t& os) {
            const TCh* cs = reinterpret_cast<const TCh*>(src);
            size_t i, j;
            for (i = 0, j = 0; i < ss && j < os; ++ i) {
                if (cs[i] >= 32 && cs[i] < 127) {
                    out[j] = cs[i];
                    ++j;
                } else if (j + 4 <= os) {
                    out[j++] = '\\';
                    oct(&out[j], cs[i]);
                    j += 3;
                } else {
                    break;
                }
            }

            os = j;
        }

        /**
         * @brief 字符转8进制表示
         * @param src 输入的buffer
         * @param ss 输入的buffer长度
         * @param out 输出缓冲区
         */
        template<typename Elem, typename Traits>
        void serialization(const void* src, size_t ss, std::basic_ostream<Elem, Traits>& out) {
            const Elem* cs = reinterpret_cast<const Elem*>(src);
            size_t i;
            for (i = 0; i < ss; ++i) {
                if (cs[i] >= 32 && cs[i] < 127) {
                    out.put(cs[i]);
                } else {
                    Elem tmp[4] = {'\\', 0, 0, 0};
                    oct(&tmp[1], cs[i]);
                    out.write(tmp, 4);
                }
            }
        }
    }
}

#endif
