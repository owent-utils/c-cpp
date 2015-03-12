/**
* @file DynamicIdxList.h
* @brief 固定下标链表<br />
* 可用于共享内存<br />
* Licensed under the MIT licenses.
*
* @file DynamicIdxList.h
* @brief 固定下标链表（支持动态增长，支持随机访问，地址可能变化，但是index不会变化）<br />
* @warning 注意：不可用于共享内存，内部采用了std::vector实现<br />
* 构造时会清空数据
* @note 测试编译器 GCC 4.4.6, 4.8.1, VC 11.0
*
* @version 1.0
* @author OWenT
* @date 2013-06-04
*
* @history
*     2013-12-25 结构重构
*/

#ifndef _UTIL_DS_DYNAMICIDXLIST_H_
#define _UTIL_DS_DYNAMICIDXLIST_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <algorithm>
#include <cstdlib>
#include <vector>
#include <limits.h>
#include <assert.h>

#include "IdxListBase.h"

namespace util
{
    namespace ds
    {
        namespace detail
        {
            template<typename Ty>
            class DynamicIdxListContainer
            {
            public:
                typedef Ty value_type;
                typedef std::vector<value_type> alloc_type;
                typedef typename alloc_type::size_type size_type;

            private:
                alloc_type m_stData;

            public:
                bool empty() const { return m_stData.empty(); }
                size_type size() const { return m_stData.size(); }

                value_type* create()
                {
                    m_stData.push_back(value_type());
                    return &m_stData.back();
                }

                void release() { m_stData.pop_back(); }

                value_type& back() { return m_stData.back(); }

                const value_type& back() const { return m_stData.back(); }

                void clear() { m_stData.clear(); }

                value_type& operator[](size_type i) { return m_stData[i]; }
                const value_type& operator[](size_type i) const { return m_stData[i]; }

                alloc_type& GetAlloc() { return m_stData; }
                const alloc_type& GetAlloc() const  { return m_stData; }
            };
        }

        /**
         * 支持随机访问的C++型链表
         * @note 目标结构体至少要有默认构造函数, 构造函数最多三个参数
         */
        template<typename TObj, typename TContainer = detail::DynamicIdxListContainer<detail::IdxListBufferNode<TObj, size_t> > >
        class DynamicIdxList: public detail::IdxListBase<TObj, TContainer>
        {

        public:
            typedef detail::IdxListBase<TObj, TContainer> base_type;
            typedef typename base_type::size_type size_type;
            typedef typename base_type::container_type container_type;
            typedef typename container_type::alloc_type alloc_type;
            typedef typename base_type::node_type node_type;
            typedef typename base_type::value_type value_type;
            typedef DynamicIdxList<TObj, TContainer> self_type;

            typedef typename base_type::iterator iterator;
            typedef typename base_type::const_iterator const_iterator;

        private:
            using base_type::GetContainer;

        public:
            using base_type::construct;

            DynamicIdxList()
            {
                construct();
            }

            /**
             * reserve操作
             * @param [in] _Count 单元元素个数，类似std::vector<T,A>::reserve
             */
            void reserve(size_type _Count)
            {
                container_type& stContainer = GetContainer();
                stContainer.GetAlloc().reserve(_Count);
            }

            /**
             * capacity操作
             * @return 单元元素个数，类似std::vector<T,A>::capacity
             */
            size_type capacity() const
            {
                container_type& stContainer = GetContainer();
                return stContainer.GetAlloc().capacity();
            }
        };
    }
}

#endif /* _UTIL_DS_DYNAMICIDXLIST_H_ */
