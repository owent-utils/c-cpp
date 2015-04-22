/**
 * @file formular_mc.h
 * @brief 公式计算（按模式）
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

#include "formular_base.h"

namespace util
{
    namespace logic
    {

        template<typename TVal, typename TC, typename TFn>
        class formular_mc : public detail::formular_base {
        public:
            typedef typename detail::formular_def<TVal, TC, TFn>::value_type value_type;
            typedef typename detail::formular_def<TVal, TC, TFn>::container_type container_type;
            typedef typename detail::formular_def<TVal, TC, TFn>::call_type call_type;
            typedef typename detail::formular_def<TVal, TC, TFn>::caller_type caller_type;
            typedef typename detail::formular_def<TVal, TC, TFn>::placeholder_type placeholder_type;

            bool init(const char* content, size_t placeholder_number = 0) {
                fn = generate_caller<TVal, TC, TFn>(content);

                ph.resize(placeholder_number, static_cast<value_type>(0));

                return !!fn;
            }

            value_type operator()(TC& container,const TFn& fn_key) {
                return fn(container, fn_key, ph);
            }

            void set_placeholder(size_t index, value_type v) {
                if (ph.size() <= index) {
                    size_t cap = ph.capacity();
                    while (cap <= index) {
                        cap <<= 1;
                    }
                    ph.resize(cap, static_cast<value_type>(0));
                }

                ph[index] = v;
            }

            value_type get_placeholder(size_t index) {
                return ph.size() > index ? ph[index] : static_cast<value_type>(0);
            }

            bool available() const { return !!fn; }
        private:
            caller_type fn;
            placeholder_type ph;
        };
    }
}