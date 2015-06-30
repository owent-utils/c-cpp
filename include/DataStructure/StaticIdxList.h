/**
* @file IdxList.h
* @brief 固定下标链表<br />
* 可用于共享内存<br />
* Licensed under the MIT licenses.
*
* @warning 注意：如果是新创建的结构，需要执行construct函数初始化
* @note 测试编译器 GCC 4.7.2, VC 11.0
*
* @version 1.0
* @author OWenT
* @date 2013-2-26
*
* @history
*     2013-02-28 增加const限定支持
*     2013-02-29 优化迭代器结构，增加下标索引访问权限
*     2013-12-25 结构优化，整体重构
*/

#ifndef _UTIL_DS_STATICIDXLIST_H_
#define _UTIL_DS_STATICIDXLIST_H_

#include <algorithm>
#include <cstdlib>
#include <assert.h>

#include "IdxListBase.h"
#include "MemPool/StaticAllocator.h"

namespace util
{
    namespace ds
    {
        namespace detail
        {
            template<typename TAlloc>
            class StaticIdxListContainer
            {
            public:
                typedef TAlloc alloc_type;
                typedef typename alloc_type::value_type value_type;
                typedef typename alloc_type::size_type size_type;

            private:
                size_type m_uAllocTop;
                alloc_type m_stData;

            public:
                bool empty() const { return 0 == m_uAllocTop; }
                size_type size() const { return m_uAllocTop; }

                value_type* create()
                {
                    if (m_uAllocTop >= m_stData.max_size())
                    {
                        return NULL;
                    }

                    value_type* pRet = m_stData.get(m_uAllocTop++);
                    ::new ((void*)pRet)value_type();
                    return pRet;
                }

                void release()
                {
                    if (m_uAllocTop > 0)
                    {
                        -- m_uAllocTop;
                    }
                }

                value_type& back() { return (m_uAllocTop > 0)? *m_stData.get(m_uAllocTop - 1): *m_stData.get(0); }

                const value_type& back() const { return (m_uAllocTop > 0)? *m_stData.get(m_uAllocTop - 1): *m_stData.get(0); }

                void clear() { m_uAllocTop = 0; }

                value_type& operator[](size_type i) { return *m_stData.get(i); }
                const value_type& operator[](size_type i) const { return *m_stData.get(i); }

                alloc_type& GetAlloc() { return m_stData; }
                const alloc_type& GetAlloc() const  { return m_stData; }
            };
        }

        /**
         * 可用于共享内存的C++型循环链表
         * @warning 注意：如果是新创建的结构，需要执行construct函数初始化,如果从共享内存恢复，无需执行
         * @warning 如果自定义内存分配器，需要分配一个冗余节点用于存储起点信息
         * @note 目标结构体至少要有默认构造函数, 构造函数最多三个参数
         * @note 内存消耗为 (sizeof(TObj) + 2 * sizeof(size_type))
         */
        template<
            typename TObj,
            size_t MAX_SIZE,
            typename TAlloc = mempool::StaticAllocator< detail::IdxListBufferNode<TObj, size_t>, MAX_SIZE>
        >
        class StaticIdxList: public detail::IdxListBase<TObj, detail::StaticIdxListContainer<TAlloc> >
        {
        public:
            typedef detail::IdxListBase<TObj, detail::StaticIdxListContainer<TAlloc> > base_type;
            typedef typename base_type::size_type size_type;
            typedef typename base_type::container_type container_type;
            typedef typename container_type::alloc_type alloc_type;
            typedef typename base_type::node_type node_type;
            typedef typename base_type::value_type value_type;
            typedef StaticIdxList<TObj, MAX_SIZE, TAlloc> self_type;

            typedef typename base_type::iterator iterator;
            typedef typename base_type::const_iterator const_iterator;

        private:
            using base_type::GetContainer;

        public:
            StaticIdxList(){}

            void construct()
            {
                container_type& stContainer = GetContainer();
                stContainer.clear();
                base_type::construct();
            }
        };
    }
}

#endif /* _UTIL_DS_STATICIDXLIST_H_ */
