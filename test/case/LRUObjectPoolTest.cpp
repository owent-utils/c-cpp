#include <cstring>

#include "frame/test_macros.h"
#include "MemPool/lru_object_pool.h"


struct test_lru_data {};
static int g_stat_lru[4] = { 0, 0, 0, 0};

struct test_lru_action: public util::mempool::lru_default_action<test_lru_data> {
    typedef util::mempool::lru_default_action<test_lru_data> base_type;
    void push(test_lru_data* obj) {
        ++g_stat_lru[0];
    }

    void pull(test_lru_data* obj) {
        ++g_stat_lru[1];
    }

    void reset(test_lru_data* obj) {
        ++g_stat_lru[2];
    }

    void gc(test_lru_data* obj) {
        ++g_stat_lru[3];
        base_type::gc(obj);
    }
};

CASE_TEST(LRUObjectPool, basic) 
{
    {
        typedef util::mempool::lru_pool<uint32_t, test_lru_data, test_lru_action> test_lru_pool_t;
        util::mempool::lru_pool_manager::ptr_t mgr = util::mempool::lru_pool_manager::create();
        test_lru_pool_t lru;
        test_lru_data* check_ptr;
        lru.init(mgr);

        memset(&g_stat_lru, 0, sizeof(g_stat_lru));

        CASE_EXPECT_TRUE(lru.push(123, new test_lru_data()));
        CASE_EXPECT_TRUE(lru.push(456, check_ptr = new test_lru_data()));
        CASE_EXPECT_TRUE(lru.push(123, new test_lru_data()));

        CASE_EXPECT_EQ(3, g_stat_lru[0]);
        CASE_EXPECT_EQ(0, g_stat_lru[1]);
        CASE_EXPECT_EQ(0, g_stat_lru[2]);
        CASE_EXPECT_EQ(0, g_stat_lru[3]);

        CASE_EXPECT_EQ(3, mgr->item_count().get());
        CASE_EXPECT_EQ(3, mgr->list_count().get());

        CASE_EXPECT_EQ(NULL, lru.pull(789));
        CASE_EXPECT_EQ(check_ptr, lru.pull(456));
        CASE_EXPECT_EQ(NULL, lru.pull(456));
        delete check_ptr;
        check_ptr = NULL;

        CASE_EXPECT_EQ(3, g_stat_lru[0]);
        CASE_EXPECT_EQ(1, g_stat_lru[1]);
        CASE_EXPECT_EQ(1, g_stat_lru[2]);
        CASE_EXPECT_EQ(0, g_stat_lru[3]);

        CASE_EXPECT_EQ(1, mgr->gc());

        CASE_EXPECT_EQ(3, g_stat_lru[0]);
        CASE_EXPECT_EQ(1, g_stat_lru[1]);
        CASE_EXPECT_EQ(1, g_stat_lru[2]);
        CASE_EXPECT_EQ(1, g_stat_lru[3]);
    }

    CASE_EXPECT_EQ(3, g_stat_lru[0]);
    CASE_EXPECT_EQ(1, g_stat_lru[1]);
    CASE_EXPECT_EQ(1, g_stat_lru[2]);
    CASE_EXPECT_EQ(2, g_stat_lru[3]);
}

CASE_TEST(LRUObjectPool, inner_gc_proc)
{
    typedef util::mempool::lru_pool<uint32_t, test_lru_data, test_lru_action> test_lru_pool_t;
    util::mempool::lru_pool_manager::ptr_t mgr = util::mempool::lru_pool_manager::create();
    test_lru_pool_t lru;
    lru.init(mgr);
    mgr->set_proc_item_count(16);
    mgr->set_proc_list_count(16);

    mgr->set_item_max_bound(32);

    memset(&g_stat_lru, 0, sizeof(g_stat_lru));
    test_lru_data* checked_ptr[32];

    for (int i = 0; i < 32; ++ i) {
        CASE_EXPECT_TRUE(lru.push(123, checked_ptr[i] = new test_lru_data()));
    }

    for (int i = 0; i < 24; ++i) {
        CASE_EXPECT_EQ(checked_ptr[31 - i], lru.pull(123));
    }

    for (int i = 0; i < 24; ++i) {
        CASE_EXPECT_TRUE(lru.push(123, checked_ptr[i + 8]));
    }

    CASE_EXPECT_EQ(56, g_stat_lru[0]);
    CASE_EXPECT_EQ(24, g_stat_lru[1]);
    CASE_EXPECT_EQ(24, g_stat_lru[2]);
    CASE_EXPECT_EQ(0, g_stat_lru[3]);

    CASE_EXPECT_TRUE(lru.push(123, new test_lru_data()));
    CASE_EXPECT_EQ(1, g_stat_lru[3]);
    mgr->set_item_max_bound(8);
    CASE_EXPECT_TRUE(lru.push(123, new test_lru_data()));
    CASE_EXPECT_EQ(8, g_stat_lru[3]);
    CASE_EXPECT_EQ(41, mgr->list_count().get());
    CASE_EXPECT_EQ(26, mgr->item_count().get());

    CASE_EXPECT_EQ(1, mgr->proc());
    CASE_EXPECT_EQ(25, mgr->list_count().get());
    CASE_EXPECT_EQ(25, mgr->item_count().get());

    CASE_EXPECT_EQ(16, mgr->proc());
    CASE_EXPECT_EQ(9, mgr->list_count().get());
    CASE_EXPECT_EQ(9, mgr->item_count().get());

    CASE_EXPECT_EQ(1, mgr->proc());
    CASE_EXPECT_EQ(8, mgr->list_count().get());
    CASE_EXPECT_EQ(8, mgr->item_count().get());

    CASE_EXPECT_EQ(0, mgr->proc());
    CASE_EXPECT_EQ(8, mgr->list_count().get());
    CASE_EXPECT_EQ(8, mgr->item_count().get());
}