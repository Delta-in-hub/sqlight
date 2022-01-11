#if !defined(__SQLIGHT__)
#define __SQLIGHT__
#include <ciso646>
#include <cstdint>
#include <tuple>

#define SQLIGHT_VERSION "0.01"

constexpr unsigned BYTEINBITS = 8;

constexpr unsigned PAGESIZE = 4096; // A page is 4096 bytes.

constexpr unsigned CACHESIZE = 4096 * 2; // Cache could contains CACHESIZE pages for maximum.

struct TableHeader
{
    uint32_t _recordSize;    // a record size in byte
    uint32_t _existsPageNum; // existed page number in the file exclude first page(header page)
    uint32_t _slotsPerPage;  // record number of one page could have, a slot for a record
    uint32_t _nextPage;      // next page could use
};

struct PageHeader
{
    // uint32_t _existsRecordNum;
    uint32_t _nextSlot;
};

constexpr uint32_t MAXRECORDSIZE = 4096 - sizeof(PageHeader) - 1;

struct Rid
{
    int _fd; // ? maybe useless ?
    uint32_t _page;
    uint32_t _slot;
    bool operator==(Rid other) const
    {
        return std::tie(_fd, _page, _slot) == std::tie(other._fd, other._page, other._slot);
    }
};

struct Pid
{
    int fd;
    uint32_t pageNum;
    bool operator==(const Pid &other) const
    {
        return std::tie(fd, pageNum) == std::tie(other.fd, other.pageNum);
    }
};

#endif // __SQLIGHT__
