/**
* @file IdxMemType.h
* @brief 基于下标的共享内存链表管理器(支持随机访问)
* Licensed under the MIT licenses.
*
* @version 1.0
* @author OWenT
* @date 2013-12-25
*
* @history
* 
*/

#ifndef _UTIL_MEMPOOL_IDXSHMMEMTYPE_H_
#define _UTIL_MEMPOOL_IDXSHMMEMTYPE_H_

#include <limits>
#include <assert.h>

#include "std/smart_ptr.h"
#include "DataStructure/StaticIdxList.h"

namespace util
{
        namespace mempool
        {
            namespace wrapper
            {
                template<typename Ty>
                class IdxShmMemTypeWrapper: public Ty
                {
                public:
                    IdxShmMemTypeWrapper(){}
                    ~IdxShmMemTypeWrapper(){}
                };
            }

        template<typename Ty, size_t MAX_SIZE>
        class IdxShmMemType
        {
        public:
            typedef wrapper::IdxShmMemTypeWrapper<Ty> value_type;
            typedef value_type* value_ptr_type;
            typedef detail::IdxListBufferNode<value_type, size_t> node_type;
            typedef StaticIdxList<
                value_type,
                MAX_SIZE,
                StaticShmAllocator<node_type, MAX_SIZE>
            > container_type;
            typedef IdxShmMemType<Ty, MAX_SIZE> self_type;

        private:
            typedef typename container_type::size_type inner_size_type;
            static container_type* m_pMemPool;

            int m_iObjectID; //!对象ID，即在DynamicIdxList中的数组下标

        public:
            virtual ~IdxShmMemType(){}

            virtual int OnResume() { return 0; };

            /**
             * 清空数据
             */
            static void ClearAll()
            {
                m_pMemPool->destruct();
            }

            //!获取对象ID
            inline int GetObjectID() const { return m_iObjectID; }

        private:
            /**
             * 恢复函数使用
             */
            struct _resume_obj
            {
                container_type& stSelf;

                _resume_obj(container_type& self): stSelf(self){}

                void operator()(typename container_type::size_type idx, value_type& stObj)
                {
                    // 重建虚函数表
                    new ((void*)&stObj)value_type();

                    // 触发恢复事件
                    stObj.OnResume();
                }
            };

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
             * @note 注意内存池中保存的数据并不是传入类型value_type，而是其智能指针 std::shared_ptr<value_type> (value_ptr_type)
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

            static value_type* Create()
            {
                value_type* pNewObj = new value_type();
                if (NULL == pNewObj)
                {
                    return NULL;
                }

                value_ptr_type ptr(pNewObj);
                inner_size_type iIdx = m_pMemPool->Create(ptr);

                ptr->m_iObjectID = iIdx;
                return pNewObj;
            }

            static value_type* GetByIdx(const int iIdx)
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

            static int DeleteByIdx(const int iIdx)
            {
                inner_size_type uInnerIdx = static_cast<inner_size_type>(iIdx);
                if (false == m_pMemPool->IsExists(uInnerIdx))
                {
                    return -2;
                }

                m_pMemPool->Remove(uInnerIdx);
                return 0;
            }
        };

        template<typename Ty, size_t MAX_SIZE>
        typename IdxShmMemType<Ty, MAX_SIZE>::container_type* IdxShmMemType<Ty, MAX_SIZE>::m_pMemPool = NULL;
    }
}
#endif /* _UTIL_MEMPOOL_IDXSHMMEMTYPE_H_ */
