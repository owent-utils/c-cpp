/**
* @file StaticAllocator.h
* @brief 共享内存分配器<br />
*                可用于共享内存成员
* Licensed under the MIT licenses.
*
* @version 1.0
* @author OWenT
* @date 2013-12-25
*
* @history
*/

#ifndef _UTIL_MEMPOOL_STATICSHMALLOCATOR_H_
#define _UTIL_MEMPOOL_STATICSHMALLOCATOR_H_

#include "STDAllocatorBase.h"

namespace util
{
    namespace mempool
    {
        /**
         * @note 注意：缓冲区指针按类型唯一
         */
        template<typename TObj, size_t MAX_SIZE>
        struct StaticShmAllocator: __StdAllocatorBase<TObj>
        {
            // 特列分配器定义
            typedef __StdAllocatorBase<TObj>                    base_alloc_type;

            // 标准分配器定义
            typedef typename base_alloc_type::value_type        value_type;
            typedef typename base_alloc_type::pointer           pointer;
            typedef typename base_alloc_type::const_pointer     const_pointer;
            typedef typename base_alloc_type::reference         reference;
            typedef typename base_alloc_type::const_reference   const_reference;
            typedef typename base_alloc_type::size_type         size_type;
            typedef typename base_alloc_type::difference_type   difference_type;
            template<typename TU> struct rebind { typedef StaticAllocator<TU, MAX_SIZE> other; };

            // 标准分配器函数
            StaticShmAllocator() {};
            StaticShmAllocator( const StaticAllocator<TObj, MAX_SIZE>& other ){};
            template<typename TU, size_t MAX_SIZE_U>
            StaticShmAllocator( const StaticAllocator<TU, MAX_SIZE_U>& other ){};

            ~StaticShmAllocator(){};

            pointer allocate(size_type uSize, const void* = 0)
            {
                return get(0);
            }

            void deallocate(pointer p, size_type n)
            {
            }

            static size_type max_size() throw() { return MAX_SIZE; }

            // 特例内存对象
            static void* m_pBuffStart;

            // 静态分配器特例函数
            pointer get(size_type i) throw()
            {
                if (i >= MAX_SIZE)
                {
                    return NULL;
                }

                return (pointer)((char*)m_pBuffStart + i * sizeof(TObj));
            }

            const_pointer get(size_type i) const throw()
            {
                if (i >= MAX_SIZE)
                {
                    return NULL;
                }

                return (const_pointer)((char*)m_pBuffStart + i * sizeof(TObj));
            }
        };

        template<typename TObj, size_t MAX_SIZE>
        void* StaticShmAllocator<TObj, MAX_SIZE>::m_pBuffStart = NULL;
    }
}

#endif /* _UTIL_MEMPOOL_STATICSHMALLOCATOR_H_ */
