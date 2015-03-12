/**
* @file STDAllocatorBase.h
* @brief 自定义内存分配器基类
* Licensed under the MIT licenses.
*
* @version 1.0
* @author OWenT
* @date 2013-6-20
*
* @history
* 
*/

#ifndef _UTIL_MEMPOOL_STDALLOCATORBASE_H_
#define _UTIL_MEMPOOL_STDALLOCATORBASE_H_

#include <cstddef>

namespace util
{
    namespace mempool
    {
        template<typename TObj>
        struct __StdAllocatorBase
        {
            // 标准分配器定义
            typedef TObj            value_type;
            typedef TObj*           pointer;
            typedef const TObj*     const_pointer;
            typedef TObj&           reference;
            typedef const TObj&     const_reference;
            typedef size_t          size_type;
            typedef std::ptrdiff_t  difference_type;

            // template<typename TU> struct rebind { typedef __StdAllocatorBase<TU> other; };

            pointer address( reference x ) const
            {
                return &x;
            }

            // 标准分配器函数
            const_pointer address( const_reference x ) const
            {
                return &x;
            }

            template<typename _Tp>
            void construct(pointer __p, const _Tp& __val)
            {
                ::new((void *)__p) TObj(__val);
            }

            void destroy(pointer __p) { __p->~TObj(); }

            // 扩展分配器函数
        };


        // 标准外部函数

        template<typename _Tp>
        inline bool operator==(const __StdAllocatorBase<_Tp>&, const __StdAllocatorBase<_Tp>&) { return true; }

        template<typename _Tp>
        inline bool operator!=(const __StdAllocatorBase<_Tp>&, const __StdAllocatorBase<_Tp>&) { return false; }
    }
}
#endif /* _UTIL_MEMPOOL_STDALLOCATORBASE_H_ */
