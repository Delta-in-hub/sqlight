#include "bitwise.h"
#include "fmt/color.h"
#include "fmt/format.h"
#include "pagedFile.h"
#include "record.h"
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

TEST(PagedFile, test)
{
    using namespace PagedFile;
    FileManager fm;
    char path[] = "./gtestPagedFiletest38963765中文💖😂.bin";

    EXPECT_TRUE(fm.isFile(path).empty());

    fm.createFile(path);
    EXPECT_TRUE(not fm.isFile(path).empty());

    Print("{}\n", fm.isFile(path));

    int fd = fm.openFile(path);
    PageManager pm;
    auto page = pm.getPage({fd, 0});
    // printHex(page->_data, 32);
    memset(page->_data, 1, 4096);
    // printHex(page->_data, 32);
    page->_dirty = true;

    auto page2 = pm.getPage({fd, 2});

    page2->_data[1] = 0x11;
    page2->_data[2] = 0xff;
    page2->_dirty = true;

    fm.closeFile(fd, pm);

    EXPECT_TRUE(not fm.isFile(path).empty());

    fd = fd = fm.openFile(path);

    page = pm.getPage({fd, 0});
    char buf[4096];
    memset(buf, 1, 4096);
    EXPECT_TRUE(memcmp(page->_data, buf, 4096) == 0);

    page2 = pm.getPage({fd, 2});
    EXPECT_EQ(page2->_data[1], 0x11);
    EXPECT_EQ(page2->_data[2], 0xff);
    EXPECT_EQ(page2->_data[3], 0x00);

    fm.closeFile(fd, pm);

    fm.deleteFile(path);
    EXPECT_TRUE(fm.isFile(path).empty());
}

TEST(RecordManger, create)
{
    char path[] = "./gtestRecordTest中文💖😂.recordbin";

    struct record
    {
        char name[300];
        int age;
        int id;
        double fnum;
    } temp;

    char buf[4096];
    auto fun = [](int cnt, char *dest) {
        std::string s(cnt, 'a');
        strcpy(dest, s.data());
    };

    temp.age = 0;
    temp.fnum = 0;
    temp.id = 0;
    fun(temp.age + 1, temp.name);
    // using namespace RecordMgr;
    auto rf = RecordMgr::RecordFileManager();
    auto rm = rf.creatTable(path, sizeof(record));
    int cnt = 0;
    for (auto i = rm.cbegin(); i != rm.cend(); ++i) // bug
    {
        cnt++;
    }
    EXPECT_EQ(cnt, 0);
    for (int i = 0; i < 100; i++)
    {
        rm.insertRecord(&temp);
        temp.age++;
        temp.fnum++;
        temp.id++;
        fun(temp.age + 1, temp.name);
    }
    cnt = 0;
    for (auto i = rm.cbegin(); i != rm.cend(); ++i) // bug
    {
        record *p = (record *)*i;
        EXPECT_EQ(p->age, cnt);
        EXPECT_EQ(p->fnum, cnt);
        EXPECT_EQ(p->id, cnt);
        fun(cnt + 1, buf);
        EXPECT_EQ(strcmp(p->name, buf), 0);
        cnt++;
    }
    EXPECT_EQ(cnt, 100);

    rf.closeTable(rm);
}

TEST(RecordManger, open)
{

    char path[] = "./gtestRecordTest中文💖😂.recordbin";

    struct record
    {
        char name[300] = {0};
        int age;
        int id;
        double fnum;
    } temp;

    char buf[4096];
    auto fun = [](int cnt, char *dest) {
        std::string s(cnt, 'a');
        strcpy(dest, s.data());
    };

    temp.age = 0;
    temp.fnum = 0;
    temp.id = 0;
    fun(temp.age + 1, temp.name);
    // using namespace RecordMgr;
    auto rf = RecordMgr::RecordFileManager();
    auto rm = rf.openTable(path);
    int cnt = 0;
    for (auto i = rm.cbegin(); i != rm.cend(); ++i)
    {
        record *p = (record *)*i;
        EXPECT_EQ(p->age, cnt);
        EXPECT_EQ(p->fnum, cnt);
        EXPECT_EQ(p->id, cnt);
        fun(cnt + 1, buf);
        EXPECT_EQ(strcmp(p->name, buf), 0);

        rm.updateRecord(i.getRid(), &temp);
        cnt++;
    }
    EXPECT_EQ(cnt, 100);
    cnt = 0;
    std::vector<Rid> del;
    for (auto i = rm.cbegin(); i != rm.cend(); ++i)
    {
        record *p = (record *)*i;
        EXPECT_TRUE(memcmp(p, &temp, sizeof(temp)) == 0);
        cnt++;
        if (cnt % 5 == 0)
            del.push_back(i.getRid());
    }
    EXPECT_EQ(cnt, 100);
    for (auto &&i : del)
    {
        rm.deleteRecord(i);
    }
    cnt = 0;
    for (auto i = rm.cbegin(); i != rm.cend(); ++i)
    {
        record *p = (record *)*i;
        EXPECT_TRUE(memcmp(p, &temp, sizeof(temp)) == 0);
        cnt++;
    }
    EXPECT_EQ(cnt, 100 - del.size());
    auto ntemp = temp;
    ntemp.id = 123;
    ntemp.age = 321;
    for (auto &&i : del)
    {
        rm.insertRecord(&ntemp);
    }
    cnt = 0;
    for (auto i = rm.cbegin(); i != rm.cend(); ++i)
    {
        record *p = (record *)*i;
        cnt++;
        if (cnt % 5 == 0)
            EXPECT_TRUE(memcmp(p, &ntemp, sizeof(temp)) == 0);
        else
            EXPECT_TRUE(memcmp(p, &temp, sizeof(temp)) == 0);
    }
    EXPECT_EQ(cnt, 100);
    rf.closeTable(rm);
}

TEST(RecordManger, delete)
{

    char path[] = "./gtestRecordTest中文💖😂.recordbin";
    auto rf = RecordMgr::RecordFileManager();
    rf.deleteTable(path);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}