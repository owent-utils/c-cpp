/**
* @file StaticAllocator.h
* @brief 静态内存分配器<br />
*                可用于共享内存成员
* Licensed under the MIT licenses.
*
* @version 1.0
* @author OWenT
* @date 2013-2-26
*
* @history
*     2013-12-25 结构变更+重构
*/

#ifndef _UTIL_MEMPOOL_STATICALLOCATOR_H_
#define _UTIL_MEMPOOL_STATICALLOCATOR_H_

#include "STDAllocatorBase.h"

namespace util
{
    namespace mempool
    {
        template<typename TObj, size_t MAX_SIZE>
        struct StaticAllocator: __StdAllocatorBase<TObj>
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
            StaticAllocator() {};
            StaticAllocator( const StaticAllocator<TObj, MAX_SIZE>& other ) {};
            template<typename TU, size_t MAX_SIZE_U>
            StaticAllocator( const StaticAllocator<TU, MAX_SIZE_U>& other ) {};

            ~StaticAllocator(){};

            pointer allocate(size_type uSize, const void* = 0)
            {
                return get(0);
            }

            void deallocate(pointer p, size_type n)
            {
            }

            size_type max_size() const throw() { return MAX_SIZE; }

            // 特例内存对象
            typedef union 
            {
                char stBuff[MAX_SIZE * sizeof(TObj)];
                char c;
                TObj stNodes[MAX_SIZE];
            } buff_type;
            buff_type stData;

            // template<typename TF, typename TT>
            // struct alias_cast_t 
            // {
            //     TF* pFrom;
            //     TT* pTo;
            // } ;

            // 静态分配器特例函数
            pointer get(size_type i) throw()
            {
                if (i >= MAX_SIZE)
                {
                    return NULL;
                }

                return &stData.stNodes[i];
                // return (pointer)(&stData.c + i * sizeof(TObj));
                // alias_cast_t<char, TObj> alias_cast;
                // alias_cast.pFrom = &stData.c + i * sizeof(TObj);
                // return alias_cast.pTo;
            }

            const_pointer get(size_type i) const throw()
            {
                if (i >= MAX_SIZE)
                {
                    return NULL;
                }

                return &stData.stNodes[i];
                // return (const_pointer)(&stData.c + i * sizeof(TObj));
                // alias_cast_t<const char, TObj> alias_cast;
                // alias_cast.pFrom = &stData.c + i * sizeof(TObj);
                // return alias_cast.pTo;
            }
        };
    }
}

#endif /* _UTIL_MEMPOOL_STATICALLOCATOR_H_ */
