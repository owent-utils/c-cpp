
#include "frame/test_macros.h"
#include "DataStructure/StaticIdxList.h"

struct static_idx_list_helper_class
{
    int m;
    static_idx_list_helper_class(): m(0){}
    static_idx_list_helper_class(int _m): m(_m){}
    static_idx_list_helper_class(double, int _m): m(_m){}
};

static bool static_idx_list_helper_count_func(size_t, static_idx_list_helper_class& obj)
{
    return obj.m == 30;
}

struct static_idx_list_helper_class_func_obj
{
    bool operator()(size_t, const static_idx_list_helper_class& obj) const
    {
        return obj.m == 20;
    }
};

CASE_TEST(StaticIdxListTest, Count)
{
    util::ds::StaticIdxList<static_idx_list_helper_class, 128> stList;
    typedef util::ds::StaticIdxList<static_idx_list_helper_class, 128>::size_type size_type;

    stList.construct();

	CASE_EXPECT_EQ((size_type)0, stList.Count());

    stList.Create();

    stList.Create(10);
    stList.Create(20);
    stList.Create(30);

    stList.Create(30.0, 20);
    stList.Create(30.0, 50);

    CASE_EXPECT_EQ((size_type)6, stList.Count());
    CASE_EXPECT_EQ((size_type)1, stList.Count(static_idx_list_helper_count_func));

    CASE_EXPECT_EQ((size_type)2, stList.Count(static_idx_list_helper_class_func_obj()));
}

struct static_idx_list_helper_class_foreach_func_obj
{
    int& m;
    static_idx_list_helper_class_foreach_func_obj(int& _m): m (_m){}
    void operator()(size_t, static_idx_list_helper_class& obj)
    {
        m += obj.m;
    }
};

CASE_TEST(StaticIdxListTest, Foreach)
{
    int sum = 0;

    util::ds::StaticIdxList<static_idx_list_helper_class, 128> stList;
    stList.construct();

    stList.Create(10);
    stList.Create(20);
    stList.Create(30);
    stList.Create(20);
    stList.Create(10);
    stList.Create(50);
    stList.Foreach(static_idx_list_helper_class_foreach_func_obj(sum));

    CASE_EXPECT_EQ(140, sum);
}

CASE_TEST(StaticIdxListTest, Create)
{
    typedef util::ds::StaticIdxList<int, 3>::size_type size_type;
    util::ds::StaticIdxList<int, 3> stList;
    stList.construct();

    size_type idx1 = stList.Create();
    size_type idx2 = stList.Create(1);
    size_type idx3 = stList.Create(2);
    size_type idx4 = stList.Create(3);

    CASE_EXPECT_EQ((size_type)0, idx1);
    CASE_EXPECT_EQ((size_type)1, idx2);
    CASE_EXPECT_EQ((size_type)2, idx3);
    size_type npos = util::ds::StaticIdxList<int, 3>::npos;
    CASE_EXPECT_EQ(npos, idx4);

    CASE_EXPECT_EQ((size_type)3, stList.Count());
}

CASE_TEST(StaticIdxListTest, Remove)
{
    typedef util::ds::StaticIdxList<int, 5>::size_type size_type;

    util::ds::StaticIdxList<int, 5> stList;
    stList.construct();

    stList.Create(4);
    stList.Create(1);
    size_type idx3 = stList.Create(2);
    stList.Create(3);
    size_type idx5 = stList.Create(5);

    CASE_EXPECT_EQ(4, *stList.Get(0));
    CASE_EXPECT_EQ(2, *stList.Get(2));

    CASE_EXPECT_EQ((size_type)5, stList.Count());

    stList.Remove(idx3);
    stList.Remove(idx3);

    CASE_EXPECT_EQ((size_type)4, stList.Count());

    idx3 = stList.Create(6);
    CASE_EXPECT_EQ((size_type)2, idx3);

    CASE_EXPECT_EQ(5, *stList.Get(idx5));
    CASE_EXPECT_EQ(6, *stList.Get(2));

    CASE_EXPECT_EQ((size_type)5, stList.Count());

    stList.Remove(0);
    stList.Remove(1);
    stList.Remove(2);
    stList.Remove(3);
    idx3 = stList.Create(7);
    idx5 = stList.Create(8);
    CASE_EXPECT_EQ((size_type)3, idx3);
    CASE_EXPECT_EQ((size_type)2, idx5);

    stList.Remove(2);
    stList.Remove(3);
    stList.Remove(4);
    idx3 = stList.Create(9);
    CASE_EXPECT_EQ((size_type)0, idx3);
}

CASE_TEST(StaticIdxListTest, EdgeCondition)
{
    typedef util::ds::StaticIdxList<int, 4> core_type;
    typedef core_type::size_type size_type;

    core_type stList;
    stList.construct();

    size_type idx1;
    stList.Create(4);
    stList.Create(1);
    stList.Create(2);
    size_type idx4 = stList.Create(3);

    // 多次删除和创建
    stList.Remove(idx4);
    idx4 = stList.Create(5);
    stList.Remove(idx4);
    idx4 = stList.Create(6);

    CASE_EXPECT_EQ((size_type)4, stList.Count());
    CASE_EXPECT_EQ(6, *stList.Get(idx4));

    // 只有一个元素（左右都是边界）
    util::ds::StaticIdxList<int, 1> stEle;
    stEle.construct();

    stEle.Create(7);
    stEle.Create(8);
    stEle.Remove(0);

    CASE_EXPECT_EQ((size_type)0, stEle.Count());

    idx1 = stEle.Create(9);
    CASE_EXPECT_EQ((size_type)0, idx1);
    CASE_EXPECT_EQ((size_type)1, stEle.Count());

    stEle.Remove(0);
    CASE_EXPECT_EQ((size_type)0, stEle.Count());

    idx1 = stEle.Create(9);
    CASE_EXPECT_EQ((size_type)0, idx1);

    // destruct后创建
    stList.Remove(0);
    idx1 = stList.begin().index();
    CASE_EXPECT_NE((size_type)0, idx1);

    stList.destruct();
    idx4 = stList.Create(10);

    CASE_EXPECT_EQ(static_cast<size_type>(0), idx4);

    // 共享内存恢复测试
    char pData[sizeof(core_type)];
    core_type* pListOri = new (pData)core_type();
    pListOri->construct();

    pListOri->Create(12);
    pListOri->Create(13);
    pListOri->Create(14);
    pListOri->Create(15);

    CASE_EXPECT_EQ(static_cast<size_t>(4), pListOri->size());

    core_type* pListRec = new (pData)core_type();
    CASE_EXPECT_EQ(static_cast<size_t>(4), pListRec->size());
    CASE_EXPECT_EQ((size_type)0, pListRec->begin().index());
    CASE_EXPECT_EQ(14, *pListRec->get(2));
}

CASE_TEST(StaticIdxListTest, Iterator)
{
    typedef util::ds::StaticIdxList<int, 10> core_type;
    typedef core_type::size_type size_type;

    core_type stList;
    stList.construct();

    typedef core_type::node_type node_type;
    stList.Create(1);
    stList.Create(2);
    stList.Create(4);
    stList.Create(8);
    stList.Create(16);
    stList.Create(32);

    // const iterator
    int iTestBit = 0;
    for(util::ds::StaticIdxList<int, 10>::const_iterator iter = stList.begin();
        iter != stList.end(); ++ iter)
    {
        iTestBit |= *iter;
    }

    CASE_EXPECT_EQ(63, iTestBit);

    // non-const iterator
    for(util::ds::StaticIdxList<int, 10>::iterator iter = stList.begin();
        iter != stList.end(); ++ iter)
    {
        (*iter) <<= 6;
        iTestBit |= *iter;
    }
    CASE_EXPECT_EQ(4095, iTestBit);
    size_type npos = core_type::npos;
    CASE_EXPECT_EQ(npos, stList.end().index());
    CASE_EXPECT_LE((size_type)0, stList.begin().index());
}
