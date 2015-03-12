#include <typeinfo>
#include "gtest/gtest.h"

#define ENABLE_MIXEDINT_MAGIC_MASK 3

#include "Algorithm/MixedInt.h"

TEST(MixedIntTest, Basic)
{
    ASSERT_TRUE(typeid(uint32_t) != typeid(mixed_uint32_t));

    {
        mixed_uint8_t a1 = 123;
        ASSERT_EQ((uint8_t)123, (uint8_t)a1);

        mixed_uint16_t a2(1234);
        ASSERT_EQ((uint16_t)1234, (uint16_t)a2);

        mixed_uint32_t a3 = 65540;
        ASSERT_EQ((uint32_t)65540, (uint32_t)a3);

        mixed_uint64_t a4 = 20000000000LL;
        ASSERT_EQ((uint64_t)20000000000LL, (uint64_t)a4);
    }

    {
        mixed_int8_t a1 = -123;
        ASSERT_EQ((int8_t)-123, (int8_t)a1);

        mixed_int16_t a2 = -1234;
        ASSERT_EQ((int16_t)-1234, (int16_t)a2);

        mixed_int32_t a3 = -65540;
        ASSERT_EQ((int32_t)-65540, (int32_t)a3);

        mixed_int64_t a4 = -20000000000LL;
        ASSERT_EQ((int64_t)-20000000000LL, (int64_t)a4);
    }


}

TEST(MixedIntTest, Compare)
{
    mixed_int32_t a2 = -65540;
    mixed_uint32_t a3 = 65540;
    

    ASSERT_TRUE(65540 == a3);
    ASSERT_TRUE(a2 == -65540);

    ASSERT_TRUE(a3 > 65536);
    ASSERT_TRUE(-70000 < a2);

    mixed_int32_t a1 = 0;
    ASSERT_TRUE(!a1);
    ASSERT_FALSE((bool)a1);

    ASSERT_TRUE(65540 >= a3);
    ASSERT_TRUE(a2 <= -65540);
}

TEST(MixedIntTest, Calculate)
{
    mixed_int32_t a2 = 100;

    ASSERT_EQ(-10, a2 - 110);
    ASSERT_EQ(150, a2 + 50);
    ASSERT_EQ(10, a2 / 10);
    ASSERT_EQ(1000, a2 * 10);

    a2 += 23;
    ASSERT_EQ(23, a2 % 100);

    a2 -= 24;
    ASSERT_EQ(99, (int32_t)a2);

    int32_t x = a2 *= 10;
    ASSERT_EQ(990, x);

    a2 /= 9;
    a2 %= 100;
    ASSERT_EQ(10, (int32_t)a2);
}

TEST(MixedIntTest, BitOperator)
{
    mixed_uint64_t a2 = 65535;
    a2 >>= 8;
    ASSERT_EQ((uint64_t)255, (uint64_t)a2);

    a2 <<= 8;
    a2 |= 0xFF;

    ASSERT_EQ((uint64_t)65535, (uint64_t)a2);

    a2 ^= 0xFF00;
    ASSERT_EQ((uint64_t)255, (uint64_t)a2);

    ASSERT_EQ((uint64_t)15, a2 >> 4);
    ASSERT_EQ((uint64_t)15, 0xF & a2);
    ASSERT_EQ((uint64_t)65535, 0xFF00 | a2);
    ASSERT_EQ((uint64_t)15, 0xF0 ^ a2);
}
