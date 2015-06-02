/**
 * @file formular_base.h
 * @brief 公式计算（基类）
 * Licensed under the MIT licenses.
 *
 * @version 1.0
 * @author OWenT
 * @date 2015-03-06
 *
 *
 * @history
 */

#pragma once

#include <stack>
#include <string>
#include <list>
#include <cmath>
#include <sstream>
#include <vector>
#include <type_traits>
#include "std/functional.h"

namespace util
{
    namespace logic
    {
        namespace detail {

            template<typename TVal, typename TC, typename TFn>
            struct formular_def {
                typedef TVal value_type;
                typedef TC container_type;
                typedef TFn call_type;
                typedef std::vector<value_type> placeholder_type;
                typedef std::function<value_type(container_type&, const call_type&, const placeholder_type&)> caller_type;

                typedef std::stack<caller_type> caller_stack_type;
                typedef std::stack<std::string> operator_stack_type;
            };

            template<typename TVal, typename TC, typename TFn>
            struct formular_calc_element_var {
                typedef typename formular_def<TVal, TC, TFn>::value_type value_type;
                typedef typename formular_def<TVal, TC, TFn>::placeholder_type placeholder_type;

                formular_calc_element_var(size_t i) :index(i){}

                value_type operator()(TC& container, const TFn& fn, const placeholder_type& placeholder) {
                    return fn(container[index]);
                }

                size_t index;
            };

            template<typename TVal, typename TC, typename TFn>
            struct formular_calc_element_placeholder {
                typedef typename formular_def<TVal, TC, TFn>::value_type value_type;
                typedef typename formular_def<TVal, TC, TFn>::placeholder_type placeholder_type;

                formular_calc_element_placeholder(size_t i) :index(i){}

                value_type operator()(TC& container, const TFn& fn, const placeholder_type& placeholder) {
                    if (index >= placeholder.size()) {
                        return static_cast<value_type>(0);
                    }

                    return static_cast<value_type>(placeholder[index]);
                }

                size_t index;
            };

            template<typename TVal, typename TC, typename TFn>
            struct formular_calc_element_const {
                typedef typename formular_def<TVal, TC, TFn>::value_type value_type;
                typedef typename formular_def<TVal, TC, TFn>::placeholder_type placeholder_type;

                formular_calc_element_const(value_type v) :data(v){}

                operator value_type() const{
                    return data;
                }

                value_type operator()(TC&, const TFn&, const placeholder_type&) {
                    return data;
                }

                value_type data;
            };

            template<typename TVal, typename TC, typename TFn>
            struct formular_calc_operator_plus {
                typedef typename formular_def<TVal, TC, TFn>::value_type value_type;
                typedef typename formular_def<TVal, TC, TFn>::placeholder_type placeholder_type;

                typedef typename formular_def<TVal, TC, TFn>::caller_type first_type;
                typedef typename formular_def<TVal, TC, TFn>::caller_type second_type;
                formular_calc_operator_plus(const first_type& l_, const second_type& r_) :l(l_), r(r_){}

                value_type operator()(TC& container, const TFn& fn, const placeholder_type& placeholder) {
                    return l(container, fn, placeholder) + r(container, fn, placeholder);
                }

                first_type l;
                second_type r;
            };

            template<typename TVal, typename TC, typename TFn>
            struct formular_calc_operator_minus {
                typedef typename formular_def<TVal, TC, TFn>::value_type value_type;
                typedef typename formular_def<TVal, TC, TFn>::placeholder_type placeholder_type;

                typedef typename formular_def<TVal, TC, TFn>::caller_type first_type;
                typedef typename formular_def<TVal, TC, TFn>::caller_type second_type;
                formular_calc_operator_minus(const first_type& l_, const second_type& r_) :l(l_), r(r_){}

                value_type operator()(TC& container, const TFn& fn, const placeholder_type& placeholder) {
                    return l(container, fn, placeholder) - r(container, fn, placeholder);
                }

                first_type l;
                second_type r;
            };

            template<typename TVal, typename TC, typename TFn>
            struct formular_calc_operator_multi {
                typedef typename formular_def<TVal, TC, TFn>::value_type value_type;
                typedef typename formular_def<TVal, TC, TFn>::placeholder_type placeholder_type;

                typedef typename formular_def<TVal, TC, TFn>::caller_type first_type;
                typedef typename formular_def<TVal, TC, TFn>::caller_type second_type;
                formular_calc_operator_multi(const first_type& l_, const second_type& r_) :l(l_), r(r_){}

                value_type operator()(TC& container, const TFn& fn, const placeholder_type& placeholder) {
                    return l(container, fn, placeholder) * r(container, fn, placeholder);
                }

                first_type l;
                second_type r;
            };

            template<typename TVal, typename TC, typename TFn>
            struct formular_calc_operator_divi {
                typedef typename formular_def<TVal, TC, TFn>::value_type value_type;
                typedef typename formular_def<TVal, TC, TFn>::placeholder_type placeholder_type;

                typedef typename formular_def<TVal, TC, TFn>::caller_type first_type;
                typedef typename formular_def<TVal, TC, TFn>::caller_type second_type;
                formular_calc_operator_divi(const first_type& l_, const second_type& r_) :l(l_), r(r_){}

                value_type operator()(TC& container, const TFn& fn, const placeholder_type& placeholder) {
                    return l(container, fn, placeholder) / r(container, fn, placeholder);
                }

                first_type l;
                second_type r;
            };

            template<typename TVal, typename TC, typename TFn>
            struct formular_calc_operator_power {
                typedef typename formular_def<TVal, TC, TFn>::value_type value_type;
                typedef typename formular_def<TVal, TC, TFn>::placeholder_type placeholder_type;

                typedef typename formular_def<TVal, TC, TFn>::caller_type first_type;
                typedef typename formular_def<TVal, TC, TFn>::caller_type second_type;
                formular_calc_operator_power(const first_type& l_, const second_type& r_) :l(l_), r(r_){}

                value_type operator()(TC& container, const TFn& fn, const placeholder_type& placeholder) {
                    return switch_pow(
                        static_cast<typename std::conditional<std::is_floating_point<value_type>::value, float, int>::type>(0),
                        l(container, fn, placeholder),
                        r(container, fn, placeholder)
                    );
                }

                value_type switch_pow(int, value_type v, value_type pv) {
                    if (0 == v) {
                        return 0;
                    }

                    if (1 == v) {
                        return 1;
                    }

                    // 二分法pwoer
                    value_type ret = 1;
                    while (pv) {
                        if (pv & 1) {
                            ret = ret * v;
                        }
                        v *= v;
                        pv >>= 1;
                    }

                    return ret;
                }

                value_type switch_pow(float, value_type v, value_type pv) {
                    using std::pow;
                    return pow(v, pv);
                }

                first_type l;
                second_type r;
            };


            class formular_base {
            public:
                typedef char opr_t[256];


            protected:
                static opr_t& get_opr_map() {
                    static opr_t ret = { 0 };
                    if (ret['+'] > 0) {
                        return ret;
                    }

                    //优先级设置
                    char opr_map[][2] = { { '+', 1 }, { '-', 1 }, { '*', 2 }, { '/', 2 }, { '^', 3 }, { '(', 100 }, { ')', 0 } };
                    const size_t max_size = sizeof(opr_map) / sizeof(char[2]);
                    for (size_t i = 0; i < max_size; i++)
                        ret[static_cast<size_t>(opr_map[i][0])] = opr_map[i][1];
                    return ret;
                }

                static bool is_dot(const char* str) {
                    return '.' == *str;
                }

                static bool is_digital(const char* str) {
                    return *str >= '0' && *str <= '9';
                }

                static bool is_number(const char* str) {
                    return is_dot(str) || is_digital(str);
                }

                static bool is_var(const char* str) {
                    return '[' == *str;
                }

                static bool is_placeholder(const char* str) {
                    return '{' == *str;
                }

                static bool is_space(const char* str) {
                    return ' ' == *str || '\t' == *str || '\r' == *str || '\n' == *str;
                }

                static bool is_operator(const char* str) {
                    opr_t& om = get_opr_map();
                    return ')' == *str || om[static_cast<size_t>(*str)] > 0;
                }

                static const char* pick_number(const char* str) {
                    const char* start = str;
                    bool dot = false;
                    while (str && *str && ((false == dot && is_dot(str)) || is_digital(str))) {
                        ++str;
                    }

                    return start == str? nullptr: str;
                }

                static const char* pick_operator(const char* str) {
                    const char* start = str;
                    // 目前所有操作符都只有一位
                    if (str && *str && is_operator(str)) {
                        ++str;
                    }

                    return start == str ? nullptr : str;
                }

                static const char* skip_space(const char* str) {
                    while (str && *str && is_space(str)) {
                        ++str;
                    }

                    return str;
                }

                static const char* pick_var(const char* str) {
                    if (nullptr == str || !is_var(str)) {
                        return nullptr;
                    }

                    ++str;
                    str = skip_space(str);
                    while (str && *str && is_digital(str)) {
                        ++str;
                    }

                    str = skip_space(str);
                    if (']' != *str) {
                        return nullptr;
                    }

                    ++str;
                    return str;
                }

                static const char* pick_placeholder(const char* str) {
                    if (nullptr == str || !is_placeholder(str)) {
                        return nullptr;
                    }

                    ++str;
                    str = skip_space(str);
                    while (str && *str && is_digital(str)) {
                        ++str;
                    }

                    str = skip_space(str);
                    if ('}' != *str) {
                        return nullptr;
                    }

                    ++str;
                    return str;
                }

                template<typename TV>
                TV str2n(const char* str, size_t len) {
                    std::stringstream ss;
                    if (len > 0) {
                        ss.write(str, len);
                    } else {
                        ss << str;
                    }

                    TV ret = 0;
                    ss >> ret;
                    return ret;
                }

                const char* pick_words(const char* str, std::list<std::string>& res) {
                    for (str = skip_space(str); str && *str; str = skip_space(str)) {
                        const char* sv = pick_number(str);
                        if (nullptr != sv) {
                            res.push_back(std::string(str, sv));
                            str = sv;
                            continue;
                        }

                        sv = pick_var(str);
                        if (nullptr != sv) {
                            res.push_back(std::string(str, sv));
                            str = sv;
                            continue;
                        }

                        sv = pick_placeholder(str);
                        if (nullptr != sv) {
                            res.push_back(std::string(str, sv));
                            str = sv;
                            continue;
                        }

                        sv = pick_operator(str);
                        if (nullptr != sv) {
                            res.push_back(std::string(str, sv));
                            str = sv;
                            continue;
                        }

                        return str;
                    }

                    return str;
                }


                template<typename TVal, typename TC, typename TFn>
                typename formular_def<TVal, TC, TFn>::caller_type generate_stack_opr(
                    const std::string& opr, 
                    typename formular_def<TVal, TC, TFn>::caller_type& l,
                    typename formular_def<TVal, TC, TFn>::caller_type& r
                ) {
                    switch (opr[0])
                    {
                        case '+': return formular_calc_operator_plus<TVal, TC, TFn>(l, r);
                        case '-': return formular_calc_operator_minus<TVal, TC, TFn>(l, r);
                        case '*': return formular_calc_operator_multi<TVal, TC, TFn>(l, r);
                        case '/': return formular_calc_operator_divi<TVal, TC, TFn>(l, r);
                        case '^': return formular_calc_operator_power<TVal, TC, TFn>(l, r);
                    }

                    return formular_calc_element_const<TVal, TC, TFn>(static_cast<TVal>(0));
                }

                template<typename TVal, typename TC, typename TFn>
                void generate_stack_call(
                    typename formular_def<TVal, TC, TFn>::caller_stack_type& caller_stack,
                    typename formular_def<TVal, TC, TFn>::operator_stack_type& opr_stack
                ) {
                    typedef typename formular_def<TVal, TC, TFn>::caller_type ret_t;

                    ret_t pr = caller_stack.top();
                    caller_stack.pop();
                    ret_t pl = caller_stack.top();
                    caller_stack.pop();

                    std::string opr_name = opr_stack.top();
                    opr_stack.pop();

                    caller_stack.push(generate_stack_opr<TVal, TC, TFn>(opr_name, pl, pr));
                }

                template<typename TVal, typename TC, typename TFn>
                typename formular_def<TVal, TC, TFn>::caller_type generate_caller(const char* str) {
                    opr_t& om = get_opr_map();

                    typedef typename formular_def<TVal, TC, TFn>::caller_type ret_t;

                    std::list<std::string> res;
                    str = pick_words(str, res);
                    if (!str || *str) {
                        return ret_t();
                    }

                    typename formular_def<TVal, TC, TFn>::caller_stack_type caller_stack;
                    typename formular_def<TVal, TC, TFn>::operator_stack_type opr_stack;

                    while (!res.empty()) {
                        std::string v = res.front();
                        res.pop_front();

                        // 常量
                        if (is_number(v.c_str())) {
                            caller_stack.push(formular_calc_element_const<TVal, TC, TFn>(str2n<TVal>(v.c_str(), v.size())));
                            continue;
                        }

                        // 需计算的变量
                        if (is_var(v.c_str())) {
                            caller_stack.push(formular_calc_element_var<TVal, TC, TFn>(str2n<TVal>(v.c_str() + 1, v.size() - 2)));
                            continue;
                        }

                        // 占位符
                        if (is_placeholder(v.c_str())) {
                            caller_stack.push(formular_calc_element_placeholder<TVal, TC, TFn>(str2n<TVal>(v.c_str() + 1, v.size() - 2)));
                            continue;
                        }

                        // 括号结束
                        if (')' == v[0]) {
                            while ('(' != opr_stack.top()[0]) {
                                generate_stack_call<TVal, TC, TFn>(caller_stack, opr_stack);
                            }

                            opr_stack.pop();

                            continue;
                        }

                        // 运算符优先级
                        if (opr_stack.empty() || om[static_cast<size_t>(v[0])] > om[static_cast<size_t>(opr_stack.top()[0])]) {
                            opr_stack.push(v);
                            continue;
                        }

                        // 结算低优先级运算符
                        while ('(' != opr_stack.top()[0] && om[static_cast<int>(v[0])] <= om[static_cast<int>(opr_stack.top()[0])]) {
                            generate_stack_call<TVal, TC, TFn>(caller_stack, opr_stack);
                        }
                        opr_stack.push(v);
                    }

                    while (!opr_stack.empty()) {
                        generate_stack_call<TVal, TC, TFn>(caller_stack, opr_stack);
                    }

                    return caller_stack.top();
                }
            };
        }
    }
}