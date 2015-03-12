/**
* @file IdxMemTypeKV.h
* @brief 基于下标的内存池Key-Value型对象管理器
* Licensed under the MIT licenses.
*
* @version 1.0
* @author OWenT
* @date 2013-12-25
*
* @history
*     2013-12-25 结构重设计
*/

#ifndef _UTIL_MEMPOOL_IDXMEMTYPEKV_H_
#define _UTIL_MEMPOOL_IDXMEMTYPEKV_H_

#include <limits>
#include <map>

#include "std/smart_ptr.h"
#include "Algorithm/Hash.h"
#include "DataStructure/DynamicIdxList.h"

namespace util
{
        namespace mempool
        {
            namespace wrapper
            {
                template<typename Ty>
                class IdxMemTypeKVWrapper: public Ty
                {
                public:
                    IdxMemTypeKVWrapper(){}
                    ~IdxMemTypeKVWrapper(){}
                };
            }

        /**
         * 键值型内存随机访问链表，建议HASH_HVAL为素数，以减少碰撞可能性
         */
        template<typename TObj, typename TKey, int HASH_HVAL>
        class IdxMemTypeKV
        {
        public:
            typedef wrapper::IdxMemTypeKVWrapper<TObj> value_type;
            typedef TKey key_type;
            typedef std::shared_ptr<value_type> value_ptr_type;
            typedef DynamicIdxList<value_ptr_type> container_type;

        private:
            typedef typename container_type::size_type inner_size_type;
            static container_type m_astMemPool;
            static std::map<key_type, inner_size_type> m_arrKeyValuePool[HASH_HVAL];

            int m_iObjectID; //!对象ID，即在DynamicIdxList中的数组下标
            key_type m_tKey;

        public:
            virtual ~IdxMemTypeKV(){}

            /**
             * 清空数据
             */
            static void ClearAll()
            {
                m_astMemPool.destruct();
                for(int i = 0; i < HASH_HVAL; ++ i)
                {
                    m_arrKeyValuePool[i].clear();
                }
            }

            /**
             * 获取对象ID
             * @return 对象ID
             */
            inline int GetObjectID() const { return m_iObjectID; }

            /**
             * 获取对象Key
             * @return 对象Key
             */
            inline key_type GetObjectKey() const { return m_tKey; }

        public:

            /**
             * 获取原始内存池(只读)
             * @note 注意内存池中保存的数据并不是传入类型Ty，而是其智能指针 std::shared_ptr<Ty> (value_ptr_type)
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

            static value_type* CreateByKey(key_type key)
            {
                int iIndex = hash::HashFNV1<int>(&key, sizeof(key), HASH_HVAL);
                iIndex = ((iIndex % HASH_HVAL) + HASH_HVAL) % HASH_HVAL;
                if (m_arrKeyValuePool[iIndex].find(key) != m_arrKeyValuePool[iIndex].end())
                {
                    return NULL;
                }

                value_type* pNewObj = new value_type();
                if (NULL == pNewObj)
                {
                    return pNewObj;
                }

                value_ptr_type ptr(pNewObj);
                inner_size_type iIdx = m_astMemPool.Create(ptr);
                ptr->m_iObjectID = iIdx;
                ptr->m_tKey = key;

                if (m_astMemPool.npos != iIdx)
                {
                    m_arrKeyValuePool[iIndex][key] = iIdx;
                    return pNewObj;
                }

                return NULL;
            }

            static value_type* GetByKey(key_type key)
            {
                int iIndex = hash::HashFNV1<int>(&key, sizeof(key), HASH_HVAL);
                iIndex = ((iIndex % HASH_HVAL) + HASH_HVAL) % HASH_HVAL;
                typename std::map<key_type, inner_size_type>::iterator iter = m_arrKeyValuePool[iIndex].find(key);
                if (iter == m_arrKeyValuePool[iIndex].end())
                {
                    return NULL;
                }

                return GetByIdx(iter->second);
            }

            static value_type* GetByIdx(int iIdx)
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

            static int DeleteByKey(key_type key)
            {
                // 计算hash值
                int iIndex = hash::HashFNV1<int>(&key, sizeof(key), HASH_HVAL);
                iIndex = ((iIndex % HASH_HVAL) + HASH_HVAL) % HASH_HVAL;
                typename std::map<key_type, inner_size_type>::iterator iter = m_arrKeyValuePool[iIndex].find(key);
                if (iter == m_arrKeyValuePool[iIndex].end())
                {
                    return -1;
                }

                // 先移除 key-value 对
                inner_size_type uInnerIdx = iter->second;
                m_arrKeyValuePool[iIndex].erase(iter);

                if (false == m_astMemPool.IsExists(uInnerIdx))
                {
                    return -2;
                }

                // 再移除节点
                m_astMemPool.Remove(uInnerIdx);
                return 0;
            }
        };

        template<typename TObj, typename TKey, int HASH_HVAL>
        typename IdxMemTypeKV<TObj, TKey, HASH_HVAL>::container_type IdxMemTypeKV<TObj, TKey, HASH_HVAL>::m_astMemPool;

        template<typename TObj, typename TKey, int HASH_HVAL>
        std::map<
            typename IdxMemTypeKV<TObj, TKey, HASH_HVAL>::key_type,
            typename IdxMemTypeKV<TObj, TKey, HASH_HVAL>::inner_size_type
        > IdxMemTypeKV<TObj, TKey, HASH_HVAL>::m_arrKeyValuePool[HASH_HVAL];
    }
}

#endif /* _UTIL_MEMPOOL_IDXMEMTYPEKV_H_ */
