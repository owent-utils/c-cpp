/**
* @file IdxMemType.h
* @brief 基于下标的内存池对象管理器
* Licensed under the MIT licenses.
*
* @version 1.0
* @author OWenT
* @date 2013-2-26
*
* @history
*    2013-12-25 结构重设计
*/

#ifndef _UTIL_MEMPOOL_IDXMEMTYPE_H_
#define _UTIL_MEMPOOL_IDXMEMTYPE_H_

#include <limits>

#include "std/smart_ptr.h"
#include "DataStructure/DynamicIdxList.h"

namespace util
{
        namespace mempool
        {
            namespace wrapper
            {
                template<typename Ty>
                class IdxMemTypeWrapper: public Ty
                {
                public:
                    IdxMemTypeWrapper(){}
                    ~IdxMemTypeWrapper(){}
                };
            }

        template<typename Ty>
        class IdxMemType
        {
        public:
            typedef wrapper::IdxMemTypeWrapper<Ty> value_type;
            typedef std::shared_ptr<value_type> value_ptr_type;
            typedef DynamicIdxList<value_ptr_type> container_type;

        private:
            typedef typename container_type::size_type inner_size_type;
            static container_type m_astMemPool;

            int m_iObjectID; //!对象ID，即在DynamicIdxList中的数组下标

        public:
            virtual ~IdxMemType(){}

            /**
             * 清空数据
             */
            static void ClearAll()
            {
                m_astMemPool.destruct();
            }

            //!获取对象ID
            inline int GetObjectID() const { return m_iObjectID; }

        public:

            /**
             * 获取原始内存池(只读)
             * @note 注意内存池中保存的数据并不是传入类型value_type，而是其智能指针 std::shared_ptr<value_type> (value_ptr_type)
             * @note 用来进行高级操作，比如迭代器枚举,count和foreach
             * @return 原始内存池
             */
            static const container_type& GetMemoryPool()
            {
                return m_astMemPool;
            }

            /**
             * 设置分配内存块连续区域个数
             * @param iCount [in] 连续区域个数
             */
            static void Reserve(int iCount)
            {
                m_astMemPool.reserve(static_cast<inner_size_type>(iCount));
            }

            /**
             * 获取分配内存块连续区域个数
             * @return 当前分配内存块连续区域个数
             */
            static int Capacity()
            {
                return static_cast<int>(m_astMemPool.capacity());
            }

        public:

            static int GetUsedObjNumber()
            {
                return static_cast<int>(m_astMemPool.size());
            }

            static int GetFreeObjNumber()
            {
                return std::numeric_limits<int>::max() - GetUsedObjNumber();
            }

            static value_type* Create()
            {
                value_type* pNewObj = new value_type();
                if (NULL == pNewObj)
                {
                    return NULL;
                }

                value_ptr_type ptr(pNewObj);
                inner_size_type iIdx = m_astMemPool.Create(ptr);

                ptr->m_iObjectID = iIdx;
                return pNewObj;
            }

            static value_type* GetByIdx(const int iIdx)
            {
                inner_size_type uInnerIdx = static_cast<inner_size_type>(iIdx);
                if (m_astMemPool.IsExists(uInnerIdx))
                    return m_astMemPool[uInnerIdx].get();

                return NULL;
            }


            static value_type* GetFirst()
            {
                if (m_astMemPool.empty())
                    return NULL;

                return (*m_astMemPool.begin()).get();
            }

            static int GetUsedHead()
            {
                return static_cast<int>(m_astMemPool.begin().index());
            }

            static int GetNextIdx(const int iIdx)
            {
                inner_size_type uInnerIdx = static_cast<inner_size_type>(iIdx);

                return static_cast<int>(m_astMemPool.GetNextIdx(uInnerIdx));
            }

            static int DeleteByIdx(const int iIdx)
            {
                inner_size_type uInnerIdx = static_cast<inner_size_type>(iIdx);
                if (false == m_astMemPool.IsExists(uInnerIdx))
                {
                    return -2;
                }

                m_astMemPool.Remove(uInnerIdx);
                return 0;
            }
        };

        template<typename Ty>
        typename IdxMemType<Ty>::container_type IdxMemType<Ty>::m_astMemPool;
    }
}

#endif /* _UTIL_MEMPOOL_IDXMEMTYPE_H_ */
