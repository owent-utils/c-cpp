/**
* @file IdxShmMemTypeKV.h
* @brief Key-Value型共享内存链表管理器(支持随机访问)
* Licensed under the MIT licenses.
*
* @version 1.0
* @author OWenT
* @date 2013-12-25
*
* @history
* 
*/

#ifndef _UTIL_MEMPOOL_IDXSHMMEMTYPEKV_H_
#define _UTIL_MEMPOOL_IDXSHMMEMTYPEKV_H_

#include <limits>
#include <map>
#include <assert.h>

#include "Algorithm/Hash.h"
#include "DataStructure/StaticIdxList.h"
#include "StaticShmAllocator.h"

namespace util
{
        namespace mempool
        {
            namespace wrapper
            {
                template<typename Ty>
                class IdxShmMemTypeKVWrapper: public Ty
                {
                public:
                    IdxShmMemTypeKVWrapper(){}
                    ~IdxShmMemTypeKVWrapper(){}
                };
            }

        /**
         * 键值型内存随机访问链表，建议HASH_HVAL为素数，以减少碰撞可能性
         */
        template<typename TObj, typename TKey, int HASH_HVAL, size_t MAX_SIZE>
        class IdxShmMemTypeKV
        {
        public:
            typedef wrapper::IdxShmMemTypeKVWrapper<TObj> value_type;
            typedef TKey key_type;
            typedef value_type* value_ptr_type;
            typedef detail::IdxListBufferNode<value_type, size_t> node_type;
            typedef StaticIdxList<
                value_type, 
                MAX_SIZE, 
                StaticShmAllocator<node_type, MAX_SIZE>
            > container_type;
            typedef IdxShmMemTypeKV<TObj, TKey, HASH_HVAL, MAX_SIZE> self_type;

        private:
            typedef typename container_type::size_type inner_size_type;
            static container_type* m_pMemPool;
            static std::map<key_type, inner_size_type> m_arrKeyValuePool[HASH_HVAL];

            int m_iObjectID; //!对象ID，即在DynamicIdxList中的数组下标
            key_type m_tKey;

        public:
            virtual ~IdxShmMemTypeKV(){}

            virtual int OnResume() { return 0; };

            /**
             * 清空数据
             */
            static void ClearAll()
            {
                m_pMemPool->destruct();
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

        private:
            static int _get_hash_val(key_type key)
            {
                int iIndex = hash::HashFNV1<int>(&key, sizeof(key), HASH_HVAL);
                return ((iIndex % HASH_HVAL) + HASH_HVAL) % HASH_HVAL;
            }

            /**
             * 恢复函数使用
             */
            struct _resume_obj
            {
                container_type& stSelf;

                _resume_obj(container_type& self): stSelf(self){}

                void operator()(typename container_type::size_type idx, value_type& stObj)
                {
                    int iIndex = self_type::_get_hash_val(stObj.GetObjectKey());

                    // 重建虚函数表
                    new ((void*)&stObj)value_type();

                    // 触发恢复事件
                    stObj.OnResume();

                    // 恢复索引表(失败有exception)
                    m_arrKeyValuePool[iIndex][stObj.GetObjectKey()] = idx;
                }
            };

            friend struct _resume_obj;

        public:

            /**
             * 获取占用的内存大小
             * @return
             */
            static size_t GetMemSize()
            {
                return sizeof(container_type) + container_type::alloc_type::max_size() * sizeof(node_type);
            }

            /**
             * 初始化构造函数
             * @param [in] pBuffStart 传入缓冲区起始地址
             * @param [in] uUsedLen 分配的内存数
             * @return 剩余缓冲区起始地址
             */
            static void* Construct(void *pBuffStart, size_t uUsedLen)
            {
                assert(uUsedLen >= GetMemSize());

                m_pMemPool = new (pBuffStart)container_type();
                container_type::alloc_type::m_pBuffStart = (char*)pBuffStart + sizeof(container_type);
                m_pMemPool->construct();

                return (char*)pBuffStart + uUsedLen;
            }

            /**
             * 共享内存恢复函数
             * @param [in] pBuffStart 传入缓冲区起始地址
             * @param [in] uUsedLen 分配的内存数
             * @return 剩余缓冲区起始地址
             */
            static void* Resume(void* pBuffStart, size_t uUsedLen)
            {
                assert(uUsedLen >= GetMemSize());

                m_pMemPool = new (pBuffStart)container_type();
                container_type::alloc_type::m_pBuffStart = (char*)pBuffStart + sizeof(container_type);
                m_pMemPool->Foreach(_resume_obj(*m_pMemPool));

                return (char*)pBuffStart + uUsedLen;
            }

            /**
             * 获取原始内存池(只读)
             * @note 注意内存池中保存的数据并不是传入类型Ty，而是其智能指针 std::shared_ptr<Ty> (value_ptr_type)
             * @note 用来进行高级操作，比如迭代器枚举,count和foreach
             * @return 原始内存池
             */
            static const container_type& GetMemoryPool()
            {
                return *m_pMemPool;
            }

        public:

            static int GetUsedObjNumber()
            {
                return static_cast<int>(m_pMemPool->size());
            }

            static int GetFreeObjNumber()
            {
                return static_cast<int>(m_pMemPool->max_size() - m_pMemPool->size());
            }

            static value_type* CreateByKey(key_type key)
            {
                int iIndex = _get_hash_val(key);
                if (m_arrKeyValuePool[iIndex].find(key) != m_arrKeyValuePool[iIndex].end())
                {
                    return NULL;
                }

                // 创建数据失败
                inner_size_type iIdx = m_pMemPool->Create();
                if (iIdx == m_pMemPool->npos)
                {
                    return NULL;
                }

                // 创建索引失败，恢复数据
                if (false == m_arrKeyValuePool[iIndex].insert(std::make_pair(key, iIdx)).second)
                {
                    m_pMemPool->Remove(iIdx);
                }

                value_type& stObj = (*m_pMemPool)[iIdx];
                stObj.m_iObjectID = iIdx;
                stObj.m_tKey = key;

                return &stObj;
            }

            static value_type* GetByKey(key_type key)
            {
                int iIndex = _get_hash_val(key);
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
                if (m_pMemPool->IsExists(uInnerIdx))
                    return m_pMemPool->get(uInnerIdx).get();

                return NULL;
            }


            static value_type* GetFirst()
            {
                if (m_pMemPool->empty())
                    return NULL;

                return m_pMemPool->begin().get();
            }

            static int GetUsedHead()
            {
                return static_cast<int>(m_pMemPool->begin().index());
            }

            static int GetNextIdx(const int iIdx)
            {
                inner_size_type uInnerIdx = static_cast<inner_size_type>(iIdx);

                return static_cast<int>(m_pMemPool->GetNextIdx(uInnerIdx));
            }

            static int DeleteByKey(key_type key)
            {
                // 计算hash值
                int iIndex = _get_hash_val(key);
                typename std::map<key_type, inner_size_type>::iterator iter = m_arrKeyValuePool[iIndex].find(key);
                if (iter == m_arrKeyValuePool[iIndex].end())
                {
                    return -1;
                }

                // 先移除 key-value 对
                inner_size_type uInnerIdx = iter->second;
                m_arrKeyValuePool[iIndex].erase(iter);

                if (false == m_pMemPool->IsExists(uInnerIdx))
                {
                    return -2;
                }

                // 再移除节点
                m_pMemPool->Remove(uInnerIdx);
                return 0;
            }
        };

        template<typename TObj, typename TKey, int HASH_HVAL, size_t MAX_SIZE>
        typename IdxShmMemTypeKV<TObj, TKey, HASH_HVAL, MAX_SIZE>::container_type* IdxShmMemTypeKV<TObj, TKey, HASH_HVAL, MAX_SIZE>::m_pMemPool = NULL;

        template<typename TObj, typename TKey, int HASH_HVAL, size_t MAX_SIZE>
        std::map<
            typename IdxShmMemTypeKV<TObj, TKey, HASH_HVAL, MAX_SIZE>::key_type,
            typename IdxShmMemTypeKV<TObj, TKey, HASH_HVAL, MAX_SIZE>::inner_size_type
        > IdxShmMemTypeKV<TObj, TKey, HASH_HVAL, MAX_SIZE>::m_arrKeyValuePool[HASH_HVAL];
    }
}

#endif /* _UTIL_MEMPOOL_IDXSHMMEMTYPEKV_H_ */
