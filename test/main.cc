#include "bitwise.h"
#include "fmt/color.h"
#include "fmt/format.h"
#include <ciso646>
#include <gtest/gtest.h>
#include <thread>

#define Print(arg, ...) fmt::print(fmt::fg(fmt::color::aqua), arg, __VA_ARGS__)
using namespace testing;
// Demonstrate some basic assertions.

TEST(BitMap, test)
{
    int arr[4];
    BitMap bmap(&arr, sizeof(arr));
    bmap.setAll();
    for (auto &&i : arr)
    {
        EXPECT_TRUE(i == -1);
    }
    bmap.resetAll();
    for (auto &&i : arr)
    {
        EXPECT_TRUE(i == 0);
    }
    bmap.set(0);
    bmap.set(8);
    bmap.set(16);
    bmap.set(32);
    bmap.set(64);
    bmap.reset(64);
    bmap.set(127);
    EXPECT_EQ(bmap.get(0), bmap.get(8));
    EXPECT_EQ(bmap.get(64), bmap.get(69));
    bmap.set(64);
    EXPECT_NE(bmap.get(64), bmap.get(69));
    EXPECT_EQ(bmap.get(16), bmap.get(127));

    Print("{}\n", "printHex(&arr, sizeof(arr));");
    printHex(&arr, sizeof(arr));

    EXPECT_TRUE(bmap.getLength() == sizeof(arr) * 8);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}