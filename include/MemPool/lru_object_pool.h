/**
* @file lru_object_pool.h
* @brief lru 算法的对象池<br />
* Licensed under the MIT licenses.
*
* @version 1.0
* @author owent
* @date 2015-12-22
*
* @history
*/

#ifndef _UTIL_MEMPOOL_LRUOBJECTPOOL_H_
#define _UTIL_MEMPOOL_LRUOBJECTPOOL_H_

#include <cstddef>
#include <stdint.h>
#include <list>

#include "std/smart_ptr.h"

#include "Lock/seq_alloc.h"

#if defined(__cplusplus) && (__cplusplus >= 201103L || \
        (defined(_MSC_VER) && (_MSC_VER == 1500 && defined (_HAS_TR1)) || (_MSC_VER > 1500 && defined(_HAS_CPP0X) && _HAS_CPP0X)) || \
        (defined(__GNUC__) && defined(__GXX_EXPERIMENTAL_CXX0X__)) \
    )
#include <unordered_map>
#define _UTIL_MEMPOOL_LRUOBJECTPOOL_MAP(...) std::unordered_map< __VA_ARGS__ >
#else
#include <map>
#define _UTIL_MEMPOOL_LRUOBJECTPOOL_MAP(...) std::map< __VA_ARGS__ >
#endif

namespace util {
    namespace mempool {

        class lru_pool_base {
        public:
            class list_type_base {
            public:
                virtual uint64_t tail_id() const = 0;
                virtual size_t size() const = 0;
                virtual bool gc() = 0;
            protected:
                list_type_base() {}
                virtual ~list_type_base() {}
            };

        protected:
            lru_pool_base(){}
            virtual ~lru_pool_base(){}
        };

        /**
        * 需要注意保证lru_pool_manager所引用的所有lru_pool仍然有效
        */
        class lru_pool_manager {
        public:
            typedef std::shared_ptr<lru_pool_manager> ptr_t;

            struct check_item_t {
                uint64_t push_id;
                std::weak_ptr<lru_pool_base::list_type_base> list_;
            };

        public:
            static ptr_t create() {
                return ptr_t(new lru_pool_manager());
            }

#define _UTIL_MEMPOOL_LRUOBJECTPOOL_SETTER_GETTER(x) \
            void set_##x(size_t v) { x##_ = v; } \
            size_t get_##x() const { return x##_; }

            _UTIL_MEMPOOL_LRUOBJECTPOOL_SETTER_GETTER(item_min_bound);
            _UTIL_MEMPOOL_LRUOBJECTPOOL_SETTER_GETTER(item_max_bound);
            _UTIL_MEMPOOL_LRUOBJECTPOOL_SETTER_GETTER(list_bound);
            _UTIL_MEMPOOL_LRUOBJECTPOOL_SETTER_GETTER(proc_list_count);
            _UTIL_MEMPOOL_LRUOBJECTPOOL_SETTER_GETTER(proc_item_count);
            _UTIL_MEMPOOL_LRUOBJECTPOOL_SETTER_GETTER(gc_list);
            _UTIL_MEMPOOL_LRUOBJECTPOOL_SETTER_GETTER(gc_item);

#undef _UTIL_MEMPOOL_LRUOBJECTPOOL_SETTER_GETTER

            /**
             * @brief 获取实例缓存数量
             * @note 如果不是非常了解这个数值的作用，请不要修改它
             */
            inline util::lock::seq_alloc_u64& item_count() { return item_count_; }
            inline const util::lock::seq_alloc_u64& item_count() const { return item_count_; }

            /**
             * @brief 获取检测队列长度
             * @note 如果不是非常了解这个数值的作用，请不要修改它
             */
            inline util::lock::seq_alloc_u64& list_count() { return list_count_; }
            inline const util::lock::seq_alloc_u64& list_count() const { return list_count_; }

            /**
             * @brief 主动GC，会触发阈值自适应
             * @return 此次调用回收的元素的个数
             */
            size_t gc() {
                // 释放速度过慢，加快每帧释放速度
                if (gc_list_ > 0) {
                    proc_list_count_ = proc_list_count_ * 13 / 10;
                }

                // 释放速度过慢，加快每帧释放速度
                if (gc_item_ > 0) {
                    proc_item_count_ = proc_item_count_ * 13 / 10;
                }

                if (gc_list_ <= 0 && gc_item_ <= 0) {
                    item_min_bound_ = (item_count_.get() + item_min_bound_) / 2;
                    item_max_bound_ = (item_count_.get() + item_max_bound_ + 1) / 2;
                    list_bound_ = (list_count_.get() + list_bound_ + 1) / 2;

                    gc_list_ = list_bound_;
                    gc_item_ = item_min_bound_;
                }

                return proc();
            }

            /**
             * @brief 定时回调
             * @return 此次调用回收的元素的个数
             */
            size_t proc() {
                if (gc_list_ <= 0 && gc_item_ <= 0) {
                    return 0;
                }

                size_t ret = 0;
                size_t left_list_num = proc_list_count_;
                size_t left_item_num = proc_item_count_;

                while (true) {
                    if (left_list_num <= 0) {
                        break;
                    }

                    if (left_item_num <= 0) {
                        break;
                    }

                    if (0 != gc_item_ && item_count_.get() <= gc_item_) {
                        gc_item_ = 0;
                    }

                    if (0 != gc_list_ && list_count_.get() <= gc_list_) {
                        gc_list_ = 0;
                    }

                    if (0 == gc_item_ && 0 == gc_list_) {
                        break;
                    }

                    if (checked_list_.empty()) {
                        gc_list_ = 0;
                        gc_item_ = 0;
                        list_count_.set(0);
                        item_count_.set(0);
                        break;
                    }

                    check_item_t checked_item = checked_list_.front();
                    checked_list_.pop_front();
                    list_count_.dec();
                    --left_list_num;

                    if (checked_item.list_.expired()) {
                        continue;
                    }

                    std::shared_ptr<lru_pool_base::list_type_base> tar_ls = checked_item.list_.lock();
                    if (!tar_ls) {
                        continue;
                    }

                    if (tar_ls->tail_id() != checked_item.push_id) {
                        continue;
                    }

                    if (tar_ls->gc()) {
                        ++ret;
                        --left_item_num;
                    }
                }

                return ret;
            }

            /**
             * @brief 添加检查列表
             */
            void push_check_list(uint64_t push_id, std::weak_ptr<lru_pool_base::list_type_base> list_) {
                checked_list_.push_back(check_item_t());
                checked_list_.back().push_id = push_id;
                checked_list_.back().list_ = list_;

                list_count_.inc();

                if (list_count_.get() > list_bound_ || item_count_.get() > item_max_bound_) {
                    inner_gc();
                }
            }

        private:
            lru_pool_manager() :item_min_bound_(0), item_max_bound_(1024),
                list_bound_(2048), proc_list_count_(16), proc_item_count_(16),
                gc_list_(0), gc_item_(0) {
                item_count_.set(0);
                list_count_.set(0);
            }

            lru_pool_manager(const lru_pool_manager&);
            lru_pool_manager& operator=(const lru_pool_manager&);

            size_t inner_gc() {
                if (gc_list_ <= 0) {
                    gc_list_ = list_bound_;
                }

                if (gc_item_ <= 0) {
                    gc_item_ = item_max_bound_;
                }

                return proc();
            }
        private:
            size_t item_min_bound_;
            size_t item_max_bound_;
            util::lock::seq_alloc_u64 item_count_;
            size_t list_bound_;
            util::lock::seq_alloc_u64 list_count_;
            size_t proc_list_count_;
            size_t proc_item_count_;
            size_t gc_list_;
            size_t gc_item_;
            std::list<check_item_t> checked_list_;
        };

        template<typename TObj>
        struct lru_default_action {
            void push(TObj* obj) {}
            void pull(TObj* obj) {}
            void reset(TObj* obj) {}
            void gc(TObj* obj) {
                delete obj;
            }
        };

        template<typename TKey, typename TObj, typename TAction = lru_default_action<TObj> >
        class lru_pool: public lru_pool_base {
        public:
            typedef TKey key_t;
            
            class list_type : public lru_pool_base::list_type_base {
            public:
                struct wrapper {
                    TObj* object;
                    uint64_t push_id;
                };

                virtual uint64_t tail_id() const {
                    if (cache_.empty()) {
                        return 0;
                    }

                    return cache_.back().push_id;
                }

                virtual size_t size() const {
                    return cache_.size();
                };

                virtual bool gc() {
                    if (cache_.empty()) {
                        return false;
                    }

                    wrapper obj = cache_.back();
                    cache_.pop_back();

                    TAction act;
                    act.gc(obj.object);

                    if(owner_->mgr_) {
                        owner_->mgr_->item_count().dec();
                    }
                    return true;
                }

                lru_pool<TKey, TObj, TAction>* owner_;
                std::list<wrapper> cache_;
            };

            typedef std::shared_ptr<list_type> list_ptr_type;
            typedef _UTIL_MEMPOOL_LRUOBJECTPOOL_MAP(key_t, list_ptr_type) cat_map_type;

        private:
            lru_pool(const lru_pool&);
            lru_pool& operator=(const lru_pool&);

        public:
            lru_pool() {
                push_id_alloc_.set(0);
            }
            virtual ~lru_pool() {
                set_manager(NULL);

                for (typename cat_map_type::iterator iter = data_.begin(); iter != data_.end(); ++ iter) {
                    if (iter->second) {
                        while (iter->second->gc());
                    }
                }
                data_.clear();
            }

            /**
             * @brief 初始化
             * @param m 所属的全局管理器。相应的事件会通知全局管理器
             */
            int init(lru_pool_manager::ptr_t m) {
                set_manager(m);
                return 0;
            }

            void set_manager(lru_pool_manager::ptr_t m) {
                size_t s = 0;
                for (typename cat_map_type::iterator iter = data_.begin(); iter !=data_.end(); ++ iter) {
                    if (iter->second) {
                        s += iter->second->size();
                    }
                }

                if (mgr_) {
                    mgr_->item_count().sub(s);
                }

                mgr_ = m;
                if (m) {
                    m->item_count().add(s);
                }
            }

            bool push(key_t id, TObj* obj) {
                list_ptr_type& list_ = data_[id];
                if (!list_) {
                    list_ = std::make_shared<list_type>();
                    if (!list_) {
                        return false;
                    }

                    list_->owner_ = this;
                }

                typename list_type::wrapper obj_wrapper;
                if (NULL == obj) {
                    return false;
                }
                obj_wrapper.object = obj;
                while (0 == (obj_wrapper.push_id = push_id_alloc_.inc()));

                // 推送node, FILO
                list_->cache_.push_front(obj_wrapper);

                TAction act;
                act.push(obj);

                if (mgr_) {
                    mgr_->item_count().inc();

                    // 推送check list
                    mgr_->push_check_list(obj_wrapper.push_id, std::dynamic_pointer_cast<lru_pool_base::list_type_base>(list_));
                }

                return true;
            }

            TObj* pull(key_t id) {
                typename cat_map_type::iterator iter = data_.find(id);
                if (iter == data_.end()) {
                    return NULL;
                }

                if (!iter->second || iter->second->cache_.empty()) {
                    return NULL;
                }

                // 拉取node, FILO
                typename list_type::wrapper obj_wrapper = iter->second->cache_.front();
                iter->second->cache_.pop_front();

                TAction act;
                act.pull(obj_wrapper.object);
                act.reset(obj_wrapper.object);

                if (mgr_) {
                    mgr_->item_count().dec();
                }

                return obj_wrapper.object;
            }
        private:
            cat_map_type data_;
            lru_pool_manager::ptr_t mgr_;
            util::lock::seq_alloc_u64 push_id_alloc_;
        };
    }
}

#endif /* _UTIL_MEMPOOL_LRUOBJECTPOOL_H_ */
