#include <time.h>

#include "gtest/gtest.h"
#include "DesignPattern/Singleton.h"

class SingletonUnitTest : public Singleton<SingletonUnitTest>
{
public:
    bool b; 
    int i; 
};

TEST(SingletonTest, Instance) 
{
	SingletonUnitTest* pl = SingletonUnitTest::Instance();
    SingletonUnitTest& pr = SingletonUnitTest::GetInstance();

    pl->b = true;
    pl->i = 1024;

    EXPECT_EQ(pl, &pr);
    EXPECT_EQ(true, pr.b);
    EXPECT_EQ(1024, pr.i);
}

