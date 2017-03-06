#include <gtest/gtest.h>

int add(int a, int b) { return a + b; }

TEST(testwork, addt)
{
    int r = add(1, 2);
    EXPECT_EQ(r, 3);
}
