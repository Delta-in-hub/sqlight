#include <gtest/gtest.h>
#include "fmt/format.h"
#include "fmt/color.h"
#include "bitwise.h"
#include <thread>

#define Print(arg, ...) fmt::print(fmt::fg(fmt::color::aqua), arg, __VA_ARGS__)
using namespace testing;
// Demonstrate some basic assertions.
TEST(Example,test)
{
    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
    // Expect equality.
    EXPECT_EQ(7 * 6, 42);
    EXPECT_FLOAT_EQ(1.111111111111111111111111111111111, 1.111111111111111111111111111111112);
    int a = -2;
    // fmt::print(fmt::fg(fmt::color::yellow),"printHex({})\n",a);
    Print("printHex({})\n", a);
    printHex(&a, sizeof(a));
}

int main(int argc, char **argv)
{
    fmt::print("{}\n",std::thread::hardware_concurrency());
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}