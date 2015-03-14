
#include "frame/test_macros.h"
#include "DataStructure/DynamicIdxList.h"
#include "DataStructure/StaticIdxList.h"


struct dynamic_idx_list_helper_class
{
    int m;
    dynamic_idx_list_helper_class(): m(0){}
    dynamic_idx_list_helper_class(int _m): m(_m){}
    dynamic_idx_list_helper_class(double, int _m): m(_m){}
};


static bool dynamic_idx_list_helper_count_func(int, dynamic_idx_list_helper_class& obj)
{
    return obj.m == 30;
}

struct dynamic_idx_list_helper_class_func_obj
{
    bool operator()(int, const dynamic_idx_list_helper_class& obj)
    {
        return obj.m == 20;
    }
};

CASE_TEST(DynamicIdxListTest, Count)
{
    size_t lnpos = util::ds::StaticIdxList<int, 10>::npos;
    size_t rnpos = util::ds::DynamicIdxList<dynamic_idx_list_helper_class>::npos;
    CASE_EXPECT_EQ(lnpos, rnpos);

    util::ds::DynamicIdxList<dynamic_idx_list_helper_class> stList;

    CASE_EXPECT_EQ((size_t)0, stList.Count());

    stList.Create();

    stList.Create(10);
    stList.Create(20);
    stList.Create(30);

    stList.Create(30.0, 20);
    stList.Create(30.0, 50);

    CASE_EXPECT_EQ((size_t)6, stList.Count());
    CASE_EXPECT_EQ((size_t)1, stList.Count(dynamic_idx_list_helper_count_func));

    CASE_EXPECT_EQ((size_t)2, stList.Count(dynamic_idx_list_helper_class_func_obj()));
}

struct dynamic_idx_list_helper_class_foreach_func_obj
{
    int& m;
    dynamic_idx_list_helper_class_foreach_func_obj(int& _m): m (_m){}
    void operator()(int, dynamic_idx_list_helper_class& obj)
    {
        m += obj.m;
    }
};

CASE_TEST(DynamicIdxListTest, Foreach)
{
    int sum = 0;

    util::ds::DynamicIdxList<dynamic_idx_list_helper_class> stList;

    stList.Create(10);
    stList.Create(20);
    stList.Create(30);
    stList.Create(20);
    stList.Create(10);
    stList.Create(50);
    stList.Foreach(dynamic_idx_list_helper_class_foreach_func_obj(sum));

    CASE_EXPECT_EQ(140, sum);
}

CASE_TEST(DynamicIdxListTest, Create)
{
    util::ds::DynamicIdxList<int> stList;

    util::ds::DynamicIdxList<int>::size_type idx1 = stList.Create();
    util::ds::DynamicIdxList<int>::size_type idx2 = stList.Create(1);
    util::ds::DynamicIdxList<int>::size_type idx3 = stList.Create(2);

    CASE_EXPECT_EQ((size_t)0, idx1);
    CASE_EXPECT_EQ((size_t)1, idx2);
    CASE_EXPECT_EQ((size_t)2, idx3);

    CASE_EXPECT_EQ((size_t)3, stList.Count());
}

CASE_TEST(DynamicIdxListTest, Remove)
{
    util::ds::DynamicIdxList<int> stList;

    stList.Create(4);
    stList.Create(1);
    util::ds::DynamicIdxList<int>::size_type idx3 = stList.Create(2);
    stList.Create(3);
    util::ds::DynamicIdxList<int>::size_type idx5 = stList.Create(5);

    CASE_EXPECT_EQ(4, *stList.Get(0));
    CASE_EXPECT_EQ(2, *stList.Get(2));

    CASE_EXPECT_EQ((size_t)5, stList.Count());

    stList.Remove(idx3);
    stList.Remove(idx3);

    CASE_EXPECT_EQ((size_t)4, stList.Count());

    idx3 = stList.Create(6);
    CASE_EXPECT_EQ((size_t)2, idx3);

    CASE_EXPECT_EQ(5, *stList.Get(idx5));
    CASE_EXPECT_EQ(6, *stList.Get(2));

    CASE_EXPECT_EQ((size_t)5, stList.Count());
}

CASE_TEST(DynamicIdxListTest, EdgeCondition)
{
    typedef util::ds::DynamicIdxList<int> core_type;
    core_type stList;

    core_type::size_type idx1 = stList.Create(4);
    stList.Create(1);
    stList.Create(2);
    core_type::size_type idx4 = stList.Create(3);

    // 多次删除和创建

    stList.Remove(idx4);
    idx4 = stList.Create(5);
    stList.Remove(idx4);
    idx4 = stList.Create(6);

    CASE_EXPECT_EQ((size_t)4, stList.Count());
    CASE_EXPECT_EQ(6, *stList.Get(idx4));

    // 只有两个元素（左右边界）
    util::ds::DynamicIdxList<int> stEle;

    idx1 = stEle.Create(7);
    stEle.Create(8);
    stEle.Remove(0);

    CASE_EXPECT_EQ((size_t)1, stEle.Count());
    CASE_EXPECT_EQ((size_t)1, stEle.begin().index());

    idx1 = stEle.Create(9);
    CASE_EXPECT_EQ((size_t)0, idx1);
    CASE_EXPECT_EQ((size_t)2, stEle.Count());

    stEle.Remove(1);
    CASE_EXPECT_EQ((size_t)1, stEle.Count());
    CASE_EXPECT_EQ((size_t)0, stEle.begin().index());

    idx1 = stEle.Create(9);
    CASE_EXPECT_EQ((size_t)1, idx1);

    // destruct后创建

    stList.Remove(0);
    stList.Remove(1);
    stList.Remove(2);
    stList.Remove(3);
    idx1 = stList.begin().index();
    CASE_EXPECT_NE((size_t)0, idx1);
    CASE_EXPECT_NE((size_t)1, idx1);
    CASE_EXPECT_NE((size_t)2, idx1);
    CASE_EXPECT_NE((size_t)3, idx1);

    idx1 = stList.Create(10);
    stList.destruct();
    idx4 = stList.Create(11);

    CASE_EXPECT_EQ((size_t)idx4, idx1);
}

CASE_TEST(DynamicIdxListTest, Iterator)
{
    typedef util::ds::DynamicIdxList<int> core_type;
    core_type stList;

    typedef core_type::node_type node_type;
    stList.Create(1);
    stList.Create(2);
    stList.Create(4);
    stList.Create(8);
    stList.Create(16);
    stList.Create(32);

    // const iterator
    int iTestBit = 0;
    for(util::ds::DynamicIdxList<int>::const_iterator iter = stList.begin();
        iter != stList.end(); ++ iter)
    {
        iTestBit |= *iter;
    }

    CASE_EXPECT_EQ(63, iTestBit);

    // non-const iterator
    for(util::ds::DynamicIdxList<int>::iterator iter = stList.begin();
        iter != stList.end(); ++ iter)
    {
        (*iter) <<= 6;
        iTestBit |= *iter;
    }
    CASE_EXPECT_EQ(4095, iTestBit);

    CASE_EXPECT_LE(stList.size(), stList.end().index());
    CASE_EXPECT_GT(stList.size(), stList.begin().index());
}
