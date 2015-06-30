/**
 * @file LockHolder.h
 * @brief 锁管理器
 * Licensed under the MIT licenses.
 *
 * @version 1.0
 * @author OWenT
 * @date 2015-06-29
 *
 * @note 实现锁的自管理操作
 *
 * @history
 */

#ifndef _UTIL_LOCK_LOCK_HOLDER_H_
#define _UTIL_LOCK_LOCK_HOLDER_H_

#pragma once

#include <cstring>

#include "DesignPattern/Noncopyable.h"

namespace util
{
    namespace lock
    {
        namespace detail
        {
            template<typename TLock>
            struct DefaultLockAction
            {
                bool operator()(TLock& lock) const
                {
                    lock.Lock();
                    return true;
                }
            };

            template<typename TLock>
            struct DefaultTryLockAction
            {
                bool operator()(TLock& lock) const
                {
                    return lock.TryLock();
                }
            };

            template<typename TLock>
            struct DefaultUnlockAction
            {
                void operator()(TLock& lock) const
                {
                    return lock.Unlock();
                }
            };

            template<typename TLock>
            struct DefaultTryUnlockAction
            {
                void operator()(TLock& lock) const
                {
                    return lock.TryUnlock();
                }
            };
        }

        template<typename TLock,
            typename TLockAct = detail::DefaultLockAction<TLock>,
            typename TUnlockAct = detail::DefaultUnlockAction<TLock>
        >
        class LockHolder : public Noncopyable
        {
        public:
            typedef TLock value_type;

            LockHolder(TLock& lock): m_pLock(&lock)
            {
                if (false == TLockAct()(lock))
                {
                    m_pLock = NULL;
                }
            }

            ~LockHolder()
            {
                if (NULL != m_pLock)
                {
                    TUnlockAct()(*m_pLock);
                }
            }

            bool IsAvailable() const {
                return NULL != m_pLock;
            }

        private:
            value_type* m_pLock;
        };
    }
}

#endif /* _UTIL_LOCK_LOCK_HOLDER_H_ */
