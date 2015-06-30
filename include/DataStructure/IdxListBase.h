/**
* @file IdxList.h
* @brief 固定下标链表基类<br />
* Licensed under the MIT licenses.
*
* @warning 注意：如果是新创建的结构，需要执行construct函数初始化
* @note 测试编译器 GCC 4.7.2, VC 11.0
*
* @version 1.0
* @author OWenT
* @date 2013-12-25
*
*
*/

#ifndef _UTIL_DS_IDXLISTBASE_H_
#define _UTIL_DS_IDXLISTBASE_H_

namespace util
{
    namespace ds
    {
        namespace detail
        {
            template<typename TObj, typename TSize>
            struct IdxListBufferNode
            {
                TSize iPreIdx;
                TSize iNextIdx;
                bool bIsInited;
                typedef union
                {
                    char strBuff[sizeof(TObj)];
                    char c;
                } obj_type;
                obj_type stObjData;
            };

            /**
             * 支持随机访问的C++型链表
             * @note 目标结构体至少要有默认构造函数, 构造函数最多三个参数
             */
            template<typename TObj, typename TContainer >
            class IdxListBase
            {
            public:
                typedef TContainer container_type;
                typedef typename container_type::size_type size_type;
                typedef typename container_type::value_type node_type;
                typedef TObj value_type;
                typedef IdxListBase<TObj, container_type> self_type;

                /** 无效节点Idx **/
                static const size_type npos = static_cast<size_type>(-1);

                /**
                 * 迭代器类型
                 */
                template<typename ITObj>
                class Iterator
                {
                public:
                    typedef Iterator<ITObj> self_type;
                    typedef typename IdxListBase<TObj, container_type>::self_type list_type;

                private:
                    mutable size_type iIndex;
                    mutable list_type* m_pListPtr;

                public:
                    Iterator(size_type index, const list_type* const pListPtr):
                        iIndex(index),
                        m_pListPtr(const_cast<list_type*>(pListPtr))
                    {
                    }

                    inline size_type index() const { return iIndex; }

                    self_type& operator++() const
                    {
                        iIndex = m_pListPtr->GetNextIdx(iIndex);

                        return const_cast<Iterator&>(*this);
                    }

                    self_type operator++(int) const
                    {
                        Iterator stRet = (*this);
                        ++ stRet;
                        return stRet;
                    }

                    self_type& operator--() const
                    {
                        iIndex = m_pListPtr->GetPreIdx(iIndex);

                        return const_cast<Iterator&>(*this);
                    }

                    self_type operator--(int) const
                    {
                        Iterator stRet = (*this);
                        -- stRet;
                        return stRet;
                    }

                    ITObj* get()
                    {
                        return (ITObj*)(&m_pListPtr->m_stData[iIndex].stObjData.c);
                    }

                    const ITObj* get() const
                    {
                        return (const ITObj*)(&m_pListPtr->m_stData[iIndex].stObjData.c);
                    }

                    friend bool operator==(const Iterator& l, const Iterator& r)
                    {
                        return l.m_pListPtr == r.m_pListPtr && l.iIndex == r.iIndex;
                    }

                    friend bool operator!=(const Iterator& l, const Iterator& r)
                    {
                        return !(l == r);
                    }

                    inline ITObj* operator->()
                    {
                        return get();
                    }

                    inline const ITObj* operator->() const
                    {
                        return get();
                    }

                    inline ITObj& operator*()
                    {
                        return *get();
                    }

                    inline const ITObj& operator*() const
                    {
                        return *get();
                    }

                    void swap(const self_type& stIter) const // never throws
                    {
                        using std::swap;

                        swap(m_pListPtr, stIter.m_pListPtr);
                        swap(iIndex, stIter.iIndex);
                    }

                };

                typedef Iterator<TObj> iterator;
                typedef const Iterator<TObj> const_iterator;

            private:
                /**
                 * 数据交换函数优化(size_type)
                 * @param [in] left
                 * @param [in] right
                 */
                inline void swap(size_type& left, size_type& right)
                {
                    left ^= right ^= left ^= right;
                }

                struct IdxListHeader
                {
                    size_type m_iFirstUsedNode;
                    size_type m_iLastUsedNode;
                    size_type m_iSize;
                };

                IdxListHeader m_stHeader;
                container_type m_stData;

            protected:
                inline container_type& GetContainer() { return m_stData; }
            public:
                inline const container_type& GetContainer() const { return m_stData; }

            private:

                size_type _create_node()
                {
                    // 初始化链表
                    if (m_stData.empty())
                    {
                        node_type* pNode = m_stData.create();
                        if (NULL == pNode)
                        {
                            return npos;
                        }

                        m_stHeader.m_iFirstUsedNode = m_stHeader.m_iLastUsedNode = 0;
                        pNode->iPreIdx = pNode->iNextIdx = npos;
                        m_stHeader.m_iSize = static_cast<size_type>(1);
                        pNode->bIsInited = true;
                        return static_cast<size_type>(0);
                    }


                    node_type* pCurNode = &m_stData[m_stHeader.m_iLastUsedNode];
                    assert(false == IsExists(pCurNode->iNextIdx));
                    assert(true == IsExists(m_stHeader.m_iLastUsedNode));

                    // 新buff
                    node_type* pNode = NULL;
                    if (pCurNode->iNextIdx >= m_stData.size())
                    {
                        pNode = m_stData.create();
                        if (NULL == pNode)
                        {
                            return npos;
                        }
                        // 添加元素，可能导致内存位置变化,所以要重定向指针位置
                        pCurNode = &m_stData[m_stHeader.m_iLastUsedNode];

                        pCurNode->iNextIdx = m_stData.size() - 1;
                        pNode->iNextIdx = npos; // 后一个是溢出节点
                    }
                    else
                    {
                        pNode = &m_stData[pCurNode->iNextIdx];
                    }

                    pNode->bIsInited = true;
                    pNode->iPreIdx = m_stHeader.m_iLastUsedNode;
                    m_stHeader.m_iLastUsedNode = pCurNode->iNextIdx;
                    ++ m_stHeader.m_iSize;

                    return m_stHeader.m_iLastUsedNode;
                }


                /**
                 * 析构函数使用
                 */
                template<typename CObj>
                struct _destruct_obj
                {
                    self_type& stSelf;

                    _destruct_obj(self_type& self): stSelf(self){}

                    void operator()(size_type idx, CObj& stObj)
                    {
                        stSelf.m_stData[idx].bIsInited = false;
                        stObj.~TObj();
                    }
                };

                /**
                 * 条件计数函数为普通函数时使用
                 */
                template<typename CObj>
                struct _count_cc_func
                {
                    size_type& counter;
                    bool (*fn)(size_type, CObj&);

                    _count_cc_func(size_type& _counter, bool (*_fn)(size_type, CObj&)):
                        counter(_counter), fn(_fn){}

                    void operator()(size_type idx, CObj& stObj)
                    {
                        counter += (*fn)(idx, stObj)? 1: 0;
                    }
                };

                /**
                 * 条件计数函数为仿函数时使用
                 */
                template<typename _F, typename CObj>
                struct _count_obj_func
                {
                    size_type& counter;
                    _F& fn;

                    _count_obj_func(size_type& _counter, _F& _fn):
                        counter(_counter), fn(_fn){}

                    void operator()(size_type idx, CObj& stObj)
                    {
                        counter += fn(idx, stObj)? 1: 0;
                    }
                };

            public:
                IdxListBase(){}

                ~IdxListBase()
                {
                    destruct();
                }

                /**
                 * 容器构造初始化
                 */
                void construct()
                {
                    m_stHeader.m_iFirstUsedNode = npos;
                    m_stHeader.m_iLastUsedNode = npos;
                    m_stHeader.m_iSize = 0;

                    m_stData.clear();
                }

                /**
                 * 调用所有对象的析构函数(可选)
                 */
                void destruct()
                {
                    // 删除所有未释放对象
                    Foreach(_destruct_obj<TObj>(*this));
                    construct();
                }

                /**
                 * 判断下标节点存在
                 * @param [in] idx idx
                 * @return 存在返回true，否则返回false
                 */
                bool IsExists(size_type idx) const
                {
                    // 超出最大个数限制
                    if (idx >= m_stData.size())
                    {
                        return false;
                    }

                    // 小于0
                    if (idx < 0)
                    {
                        return false;
                    }

                    // flag 检查
                    return m_stData[idx].bIsInited;
                }

                /**
                 * 获取下一个元素的下标
                 * @param [in] idx 当前元素下标
                 * @return 存在返回下一个元素下标，不存在返回  npos
                 */
                size_type GetNextIdx(size_type idx) const
                {
                    size_type iRet = npos;

                    if (IsExists(idx))
                    {
                        iRet = m_stData[idx].iNextIdx;
                    }

                    return IsExists(iRet)? iRet: npos;

                }

                /**
                 * 获取上一个元素的下标
                 * @param [in] idx 当前元素下标
                 * @return 存在返回上一个元素下标，不存在返回npos
                 */
                size_type GetPreIdx(size_type idx) const
                {
                    size_type iRet = npos;

                    if (IsExists(idx))
                    {
                        iRet = m_stData[idx].iPreIdx;
                    }

                    return IsExists(iRet)? iRet: npos;
                }

                /**
                 * 按Idx获取节点
                 * @param [in] idx
                 * @return 存在则返回数据迭代器，不存在则返回end迭代器
                 */
                inline iterator Get(size_type idx)
                {
                    return get(idx);
                }

                /**
                 * 按Idx获取节点(const)
                 * @param [in] idx
                 * @return 存在则返回数据迭代器，不存在则返回end迭代器
                 */
                inline const_iterator Get(size_type idx) const
                {
                    return get(idx);
                }

                /**
                 * 按Idx获取节点(const)
                 * @param [in] idx
                 * @return 存在则返回数据引用
                 */
                inline TObj& operator[](size_type idx) { return *get(idx); };

                /**
                 * 按Idx获取节点(const)
                 * @param [in] idx
                 * @return 存在则返回数据常量引用
                 */
                inline const TObj& operator[](size_type idx) const { return *get(idx); };

                /**
                 * 创建节点，返回idx
                 * @return 新节点的idx
                 */
                size_type Create()
                {
                    size_type ret = _create_node();

                    if (npos != ret)
                    {
                        new ((void*) &m_stData[ret].stObjData.c)TObj();
                    }

                    return ret;
                }

                /**
                 * 创建节点，返回idx
                 * @param [in] param1 构造函数参数1
                 * @return 新节点的idx
                 */
                template<typename _TP1>
                size_type Create(const _TP1& param1)
                {
                    size_type ret = _create_node();

                    if (npos != ret)
                    {
                        new ((void*) &m_stData[ret].stObjData.c)TObj(param1);
                    }

                    return ret;
                }

                /**
                 * 创建节点，返回idx
                 * @param [in] param1 构造函数参数1
                 * @param [in] param2 构造函数参数2
                 * @return 新节点的idx
                 */
                template<typename _TP1, typename _TP2>
                size_type Create(const _TP1& param1, const _TP2& param2)
                {
                    size_type ret = _create_node();

                    if (npos != ret)
                    {
                        new ((void*) &m_stData[ret].stObjData.c)TObj(param1, param2);
                    }

                    return ret;
                }

                /**
                 * 创建节点，返回idx
                 * @param [in] param1 构造函数参数1
                 * @param [in] param2 构造函数参数2
                 * @param [in] param3 构造函数参数3
                 * @return 新节点的idx
                 */
                template<typename _TP1, typename _TP2, typename _TP3>
                size_type Create(const _TP1& param1, const _TP2& param2, const _TP3& param3)
                {
                    size_type ret = _create_node();

                    if (npos != ret)
                    {
                        new ((void*) &m_stData[ret].stObjData.c)TObj(param1, param2, param3);
                    }

                    return ret;
                }

                /**
                 * 移除一个元素
                 * @param [in] idx 下标
                 */
                void Remove(size_type idx)
                {
                    using std::swap;

                    // 不存在直接返回
                    if(!IsExists(idx))
                    {
                        return;
                    }

                    size_type iPreIdx = m_stData[idx].iPreIdx;
                    size_type iNextIdx = m_stData[idx].iNextIdx;
                    size_type iFreeFirst = m_stData[m_stHeader.m_iLastUsedNode].iNextIdx;

                    node_type stTmpPreNode, stTmpNextNode;
                    node_type *pPreNode, *pNextNode, *pFirstFreeNode, *pLastUsedNode;

                    // ================ 从占用节点移除  ================
                    // 前置节点buff存在
                    if (iPreIdx >= 0 && iPreIdx < m_stData.size())
                    {
                        pPreNode = &m_stData[iPreIdx];
                    }
                    else // 否则设为虚拟节点
                    {
                        pPreNode = &stTmpPreNode;
                        pPreNode->iNextIdx = idx;
                        pPreNode->iPreIdx = npos;
                    }

                    // 后置节点buff存在
                    if (iNextIdx >= 0 && iNextIdx < m_stData.size())
                    {
                        pNextNode = &m_stData[iNextIdx];
                    }
                    else  // 否则设为虚拟节点
                    {
                        pNextNode = &stTmpNextNode;
                        pNextNode->iNextIdx = npos;
                        pNextNode->iPreIdx = idx;
                    }

                    // 把当前节点剔除出占用链表
                    swap(pPreNode->iNextIdx, m_stData[idx].iNextIdx);
                    swap(pNextNode->iPreIdx, m_stData[idx].iPreIdx);

                    // 恢复最后一个节点
                    if (idx == m_stHeader.m_iLastUsedNode)
                    {
                        m_stHeader.m_iLastUsedNode = iPreIdx;
                    }

                    // 恢复第一个节点
                    if (idx == m_stHeader.m_iFirstUsedNode)
                    {
                        m_stHeader.m_iFirstUsedNode = iNextIdx;
                    }

                    // ================ 插入到空闲节点 ================
                    // 空闲节点buff存在
                    if (iFreeFirst >= 0 && iFreeFirst < m_stData.size())
                    {
                        pFirstFreeNode = &m_stData[iFreeFirst];
                    }
                    else  // 否则设为虚拟节点
                    {
                        pFirstFreeNode = &stTmpNextNode;
                        pFirstFreeNode->iNextIdx = npos;
                        pFirstFreeNode->iPreIdx = m_stHeader.m_iLastUsedNode;
                    }

                    // 最后使用节点buff存在
                    if (m_stHeader.m_iLastUsedNode >= 0 && m_stHeader.m_iLastUsedNode < m_stData.size())
                    {
                        pLastUsedNode = &m_stData[m_stHeader.m_iLastUsedNode];
                    }
                    else  // 否则设为虚拟节点
                    {
                        pLastUsedNode = &stTmpPreNode;
                        pLastUsedNode->iNextIdx = iFreeFirst;
                        pLastUsedNode->iPreIdx = npos;
                    }

                    // 插入到空闲节点
                    swap(pLastUsedNode->iNextIdx, m_stData[idx].iNextIdx);
                    swap(pFirstFreeNode->iPreIdx, m_stData[idx].iPreIdx);

                    // ================ 节点数据析构  ================
                    // 节点行为
                    m_stData[idx].bIsInited = false;

                    // 执行析构
                    TObj* pRemovedDataSect = (TObj*) (&m_stData[idx].stObjData.c);
                    pRemovedDataSect->~TObj();
                    // 计数减一
                    -- m_stHeader.m_iSize;

                    // 释放缓冲区内存
                    while (false == m_stData.empty() && false == m_stData.back().bIsInited)
                    {
                        m_stData.release();
                    }
                }

                /**
                 * 是否为空
                 * @return 为空返回true
                 */
                inline bool IsEmpty() const { return empty(); }

                // ===============================
                // =====        迭代器 操作                  =====
                // ===============================
                iterator get(size_type idx)
                {
                    if (!IsExists(idx))
                    {
                        return iterator(npos, this);
                    }

                    return iterator(idx, this);
                }

                const_iterator get(size_type idx) const
                {
                    if (!IsExists(idx))
                    {
                        return const_iterator(npos, this);
                    }

                    return const_iterator(idx, this);
                }

                iterator begin()
                {
                    return get(m_stHeader.m_iFirstUsedNode);
                }

                const_iterator begin() const
                {
                    return get(m_stHeader.m_iFirstUsedNode);
                }

                iterator end()
                {
                    return get(npos);
                }

                const_iterator end() const
                {
                    return get(npos);
                }

                size_t size() const
                {
                    return static_cast<size_t>(Count());
                }

                iterator erase(iterator iter)
                {
                    size_type idx = iter.index();
                    ++iter;

                    Remove(idx);

                    return iter;
                }

                inline bool empty() const
                {
                    return size() == 0;
                }

                // 高级功能
                // ===============================
                // =====       Lambda 操作             =====
                // ===============================


                /**
                 * foreach 操作
                 * @param fn 执行仿函数，参数必须为 (size_type, TObj&)
                 */
                template<typename _F>
                void Foreach(_F fn)
                {
                    iterator iter = begin();
                    iterator itEnd = end();
                    while (iter != itEnd)
                    {
                        fn(iter.index(), *iter);
                        ++ iter;
                    }
                }

                /**
                 * const foreach 操作
                 * @param fn 执行仿函数，参数必须为 (size_type, TObj&)
                 */
                template<typename _F>
                void Foreach(_F fn) const
                {
                    const_iterator iter = begin();
                    const_iterator itEnd = end();
                    while (iter != itEnd)
                    {
                        fn(iter.index(), *iter);
                        ++ iter;
                    }
                }

                /**
                 * foreach 操作
                 * @param fn 执行函数，参数必须为 (size_type, TObj&)
                 */
                template<typename _R>
                void Foreach(_R (*fn)(size_type, TObj&))
                {
                    iterator iter = begin();
                    iterator itEnd = end();
                    while (iter != itEnd)
                    {
                        (*fn)(iter.index(), *iter);
                        ++ iter;
                    }
                };

                /**
                 * const foreach 操作
                 * @param fn 执行函数，参数必须为 (size_type, TObj&)
                 */
                template<typename _R>
                void Foreach(_R (*fn)(size_type, const TObj&)) const
                {
                    const_iterator iter = begin();
                    const_iterator itEnd = end();
                    while (iter != itEnd)
                    {
                        (*fn)(iter.index(), *iter);
                        ++ iter;
                    }
                };

                /**
                 * 获取元素个数
                 * @return 元素个数
                 */
                size_type Count() const { return m_stHeader.m_iSize; };

            public:
                /**
                 * 获取符合条件的元素个数
                 * @param [in] fn 条件函数
                 * @return 符合条件的元素个数
                 */
                size_type Count(bool (*fn)(size_type, const TObj&)) const
                {
                    size_type iRet = 0;

                    Foreach(_count_cc_func<const TObj>(iRet, fn));

                    return iRet;
                };

                /**
                 * 获取符合条件的元素个数
                 * @param [in] fn 条件函数
                 * @return 符合条件的元素个数
                 */
                size_type Count(bool (*fn)(size_type, TObj&))
                {
                    size_type iRet = 0;

                    Foreach(_count_cc_func<TObj>(iRet, fn));

                    return iRet;
                };

            public:
                /**
                 * 获取符合条件的元素个数
                 * @param [in] fn 条件仿函数
                 * @return 符合条件的元素个数
                 */
                template<typename _F>
                size_type Count(_F fn) const
                {
                    size_type iRet = 0;

                    Foreach(_count_obj_func<_F, const TObj>(iRet, fn));

                    return iRet;
                };

                /**
                 * 获取符合条件的元素个数
                 * @param [in] fn 条件仿函数
                 * @return 符合条件的元素个数
                 */
                template<typename _F>
                size_type Count(_F fn)
                {
                    size_type iRet = 0;

                    Foreach(_count_obj_func<_F, TObj>(iRet, fn));

                    return iRet;
                };

            };
        }
    }
}

#endif /* _UTIL_DS_IDXLISTBASE_H_ */
