#include <time.h>
#include <iostream>
#include <algorithm>

#include "frame/test_macros.h"
#include "Logic/AttributeManager.h"

enum EN_SAMPLE_ATTR
{
    ESA_UNKNOWN = 0,
    ESA_STRENTH = 1,
    ESA_BLABLAB = 2,
    ESA_BASIC_ATTACK = 3,
    ESA_ATTACK = 4,
    ESA_MAX_HP = 5,
};

struct AttributeManagerValidSample
{
    typedef util::logic::AttributeManager<ESA_MAX_HP + 1, AttributeManagerValidSample, int> mt;

    static void GenAttrFormulaMap(mt::formula_builder_type& stFormulas)
    {
        using namespace util::logic::detail;

        /* 公式：最大生命 = 力量 * 2 + BLABLABLA */
        stFormulas[ESA_MAX_HP] = stFormulas[ESA_STRENTH] * 2 + _<mt>(ESA_BLABLAB);
        /* 公式：攻击力 = 100 * 基础攻击 - 力量 */
        stFormulas[ESA_UNKNOWN] = stFormulas[ESA_ATTACK] = 100 * _<mt>(ESA_BASIC_ATTACK) - _<mt>(ESA_STRENTH) + _<mt>(ESA_STRENTH) * _<mt>(ESA_BASIC_ATTACK);
        /* 公式：基础攻击 = BLABLABLA / 5 */
        stFormulas[ESA_BASIC_ATTACK] = _<mt>(ESA_BLABLAB) / 5;

    }
};


struct AttributeManagerInvalidSample
{
    typedef util::logic::AttributeManager<ESA_MAX_HP + 1, AttributeManagerInvalidSample, int> mt;

    static void GenAttrFormulaMap(mt::formula_builder_type& stFormulas)
    {
        using namespace util::logic::detail;

        /* 公式：最大生命 = 2 * 力量 + BLABLABLA / 基础攻击 */
        stFormulas[ESA_MAX_HP] = 2 * stFormulas[ESA_STRENTH] + _<mt>(ESA_BLABLAB) / stFormulas[ESA_BASIC_ATTACK];
        /* 公式：ESA_UNKNOWN = 攻击力 = 100 * 基础攻击 - 力量 + 力量 * 基础攻击 */
        stFormulas[ESA_UNKNOWN] = stFormulas[ESA_ATTACK] = 100 * _<mt>(ESA_BASIC_ATTACK) - stFormulas[ESA_STRENTH] + stFormulas[ESA_STRENTH] * stFormulas[ESA_BASIC_ATTACK];
        /* 公式：基础攻击 = BLABLABLA / 5 + 最大生命 */
        stFormulas[ESA_BASIC_ATTACK] = _<mt>(ESA_BLABLAB) / 5 + stFormulas[ESA_MAX_HP];

        /* 公式循环：ESA_BLABLAB = ESA_BLABLAB + 最大生命 / 100 */
        stFormulas[ESA_BLABLAB] = stFormulas[ESA_BLABLAB]() + stFormulas[ESA_MAX_HP] / 100;

        /* 公式循环：力量 = 最大生命 / 10 - 基础攻击 * ESA_BLABLAB */
        stFormulas[ESA_STRENTH] = _<mt>(ESA_MAX_HP) / 10 - stFormulas[ESA_BASIC_ATTACK] * stFormulas[ESA_BLABLAB];

        /* 本来是测打印所有环的，但是这个关系式，我晕了。目测貌似所有的环都正确输出了 */
    }
};


void _check_value()
{
    AttributeManagerValidSample::mt foo;
    // 初始置空
    foo.Construct();
    foo[ESA_STRENTH] = 1000;
    foo[ESA_BLABLAB] = 512;

    // ================== 数量检查 ==================
    // 检查被依赖关系
    AttributeManagerValidSample::mt::attr_attach_list_type stList;

    // 依赖 ESA_STRENTH 的属性
    foo[ESA_STRENTH].GetAttachedAttributes(stList, false);
    CASE_EXPECT_EQ((size_t)3, stList.size());
    CASE_EXPECT_FALSE(stList.end() == std::find(stList.begin(), stList.end(), ESA_UNKNOWN));
    CASE_EXPECT_FALSE(stList.end() == std::find(stList.begin(), stList.end(), ESA_ATTACK));
    CASE_EXPECT_FALSE(stList.end() == std::find(stList.begin(), stList.end(), ESA_MAX_HP));

    // 依赖 ESA_BLABLAB 的属性
    stList.clear();
    foo[ESA_BLABLAB].GetAttachedAttributes(stList, false);
    CASE_EXPECT_EQ((size_t)2, stList.size());
    CASE_EXPECT_FALSE(stList.end() == std::find(stList.begin(), stList.end(), ESA_BASIC_ATTACK));
    CASE_EXPECT_FALSE(stList.end() == std::find(stList.begin(), stList.end(), ESA_MAX_HP));

    // 依赖 ESA_BLABLAB 的属性 - Recursion
    stList.clear();
    foo[ESA_BLABLAB].GetAttachedAttributes(stList, true);
    CASE_EXPECT_EQ((size_t)4, stList.size());
    CASE_EXPECT_FALSE(stList.end() == std::find(stList.begin(), stList.end(), ESA_UNKNOWN));
    CASE_EXPECT_FALSE(stList.end() == std::find(stList.begin(), stList.end(), ESA_BASIC_ATTACK));
    CASE_EXPECT_FALSE(stList.end() == std::find(stList.begin(), stList.end(), ESA_ATTACK));
    CASE_EXPECT_FALSE(stList.end() == std::find(stList.begin(), stList.end(), ESA_MAX_HP));

    // 依赖 ESA_ATTACK 的属性
    stList.clear();
    foo[ESA_ATTACK].GetAttachedAttributes(stList, true);
    CASE_EXPECT_EQ((size_t)0, stList.size());

    // 依赖 ESA_UNKNOWN 的属性
    stList.clear();
    foo[ESA_UNKNOWN].GetAttachedAttributes(stList, true);
    CASE_EXPECT_EQ((size_t)0, stList.size());

    // 检查参数表
    AttributeManagerValidSample::mt::attr_attach_set_type stSet;

    // ESA_STRENTH 的关联参数
    foo[ESA_STRENTH].GetAttachAttributes(stSet, true);
    CASE_EXPECT_EQ((size_t)0, stSet.size());

    // ESA_MAX_HP 的关联参数
    stSet.clear();
    foo[ESA_MAX_HP].GetAttachAttributes(stSet, true);
    CASE_EXPECT_EQ((size_t)2, stSet.size());
    CASE_EXPECT_TRUE(stSet.find(ESA_STRENTH) != stSet.end());
    CASE_EXPECT_TRUE(stSet.find(ESA_BLABLAB) != stSet.end());

    // ESA_ATTACK 的关联参数
    stSet.clear();
    foo[ESA_ATTACK].GetAttachAttributes(stSet, false);
    CASE_EXPECT_EQ((size_t)2, stSet.size());
    CASE_EXPECT_TRUE(stSet.find(ESA_STRENTH) != stSet.end());
    CASE_EXPECT_TRUE(stSet.find(ESA_BASIC_ATTACK) != stSet.end());

    // ESA_ATTACK 的关联参数 - Recursion
    stSet.clear();
    foo[ESA_ATTACK].GetAttachAttributes(stSet, true);
    CASE_EXPECT_EQ((size_t)3, stSet.size());
    CASE_EXPECT_TRUE(stSet.find(ESA_STRENTH) != stSet.end());
    CASE_EXPECT_TRUE(stSet.find(ESA_BLABLAB) != stSet.end());
    CASE_EXPECT_TRUE(stSet.find(ESA_BASIC_ATTACK) != stSet.end());


    // ================== 值验证 ==================
    CASE_EXPECT_EQ(foo[ESA_UNKNOWN], foo[ESA_ATTACK]);
    CASE_EXPECT_EQ(1000, foo[ESA_STRENTH]);
    CASE_EXPECT_EQ(512, foo[ESA_BLABLAB]);
    CASE_EXPECT_EQ(foo[ESA_BLABLAB] / 5, foo[ESA_BASIC_ATTACK]);
    CASE_EXPECT_EQ(100 * foo[ESA_BASIC_ATTACK] - foo[ESA_STRENTH] + foo[ESA_STRENTH] * foo[ESA_BASIC_ATTACK], foo[ESA_ATTACK]);
    CASE_EXPECT_EQ(foo[ESA_STRENTH] * 2 + foo[ESA_BLABLAB], foo[ESA_MAX_HP]);
}

CASE_TEST(AttributeManager, ValidCheck)
{

    CASE_EXPECT_TRUE(AttributeManagerValidSample::mt::CheckValid());

    if (false == AttributeManagerValidSample::mt::CheckValid())
    {
        return;
    }

    _check_value();
}

CASE_TEST(AttributeManager, InvalidCheck)
{
    CASE_EXPECT_FALSE(AttributeManagerInvalidSample::mt::CheckValid());
    AttributeManagerInvalidSample::mt::PrintInvalidLoops(std::cout);
}

CASE_TEST(AttributeManager, Serialize)
{
    std::stringstream ss;
    AttributeManagerValidSample::mt::Serialize(std::cout);
    AttributeManagerValidSample::mt::Serialize(ss);
    AttributeManagerValidSample::mt::ClearAllFormula();
    AttributeManagerValidSample::mt::Unserialize(ss);

    _check_value();
}
