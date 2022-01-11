#if !defined(__SQLIGHT_RECORD__)
#define __SQLIGHT_RECORD__

#include "bitwise.h"
#include "pagedFile.h"
// Record Manager
namespace RecordMgr
{

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

/**
 * @brief Record manager of a table
 *
 */
class RecordManager
{
  private:
    int _fd;
    PagedFile::PageManager *_pm;
    TableHeader _th;

    void setFileHeader(uint32_t _existsPageNum, uint32_t _nextPage)
    {
        this->_th._existsPageNum = _existsPageNum;
        this->_th._nextPage = _nextPage;
        auto p = _pm->getPage({_fd, 0});
        auto th = reinterpret_cast<TableHeader *>(p->_data);
        if (th->_existsPageNum != _existsPageNum or th->_nextPage != _nextPage)
            p->_dirty = true;
        th->_existsPageNum = _existsPageNum;
        th->_nextPage = _nextPage;
    }

    Rid getFreeSlot()
    {
        if (_th._existsPageNum == 0 and _th._nextPage == 1)
        {
            setFileHeader(1, 1);
        }
        Rid rid;
        rid._fd = this->_fd;
        auto p = _pm->getPage({_fd, _th._nextPage});
        auto ph = reinterpret_cast<PageHeader *>(p->_data);
        auto bm = BitMap(p->_data + sizeof(PageHeader), ceil(_th._slotsPerPage, BYTEINBITS));
        if (ph->_nextSlot < _th._slotsPerPage)
        {
            rid._page = this->_th._nextPage;
            rid._slot = ph->_nextSlot;
            bm.set(rid._slot);
            ph->_nextSlot = bm.nextBit(ph->_nextSlot + 1);
            p->_dirty = true;
        }
        else
        {
            // this page is full
            auto npid = _th._nextPage; // bug fixed
            while (true)
            {
                auto np = _pm->getPage({_fd, ++npid});
                auto nph = reinterpret_cast<PageHeader *>(np->_data);
                if (nph->_nextSlot < _th._slotsPerPage)
                {
                    rid._page = npid;
                    rid._slot = nph->_nextSlot;
                    auto bm = BitMap(np->_data + sizeof(PageHeader), ceil(_th._slotsPerPage, BYTEINBITS));
                    bm.set(nph->_nextSlot);
                    nph->_nextSlot = bm.nextBit(nph->_nextSlot + 1);
                    setFileHeader(std::max(_th._existsPageNum, rid._page), rid._page);
                    np->_dirty = true;
                    break;
                }
            }
        }
        return rid;
    }

    // ?? need to shrink to fit
    void deleteSlot(Rid r)
    {
        auto p = _pm->getPage({r._fd, r._page});
        auto ph = reinterpret_cast<PageHeader *>(p->_data);
        auto bm = BitMap(p->_data + sizeof(PageHeader), ceil(_th._slotsPerPage, BYTEINBITS));
        assert(bm.get(r._slot));
        ph->_nextSlot = std::min(ph->_nextSlot, r._slot);
        bm.reset(r._slot);
        p->_dirty = true;
        setFileHeader(_th._existsPageNum, std::min(_th._nextPage, r._page));
    }

    const uint8_t *readSlot(Rid r) const
    {
        assert(r._page >= 1);
        auto p = _pm->getPage({r._fd, r._page});
        auto ph = reinterpret_cast<PageHeader *>(p->_data);
        auto bm = BitMap(p->_data + sizeof(PageHeader), ceil(_th._slotsPerPage, BYTEINBITS));
        assert(bm.get(r._slot));
        const uint8_t *pointer =
            p->_data + sizeof(PageHeader) + ceil(_th._slotsPerPage, BYTEINBITS) + _th._recordSize * r._slot;
        return pointer;
    }

    void writeSlot(Rid r, const uint8_t *data)
    {
        assert(r._page >= 1);
        auto p = _pm->getPage({r._fd, r._page});
        auto bm = BitMap(p->_data + sizeof(PageHeader), ceil(_th._slotsPerPage, BYTEINBITS));
        assert(bm.get(r._slot));
        uint8_t *pointer =
            p->_data + sizeof(PageHeader) + ceil(_th._slotsPerPage, BYTEINBITS) + _th._recordSize * r._slot;
        p->_dirty = true;
        memcpy(pointer, data, _th._recordSize);
    }

  public:
    RecordManager() = delete;
    RecordManager(int fd, PagedFile::PageManager *pm, TableHeader th) : _pm(pm), _fd(fd), _th(th)
    {
        ;
    }

    // shoudl call RecordFileManager::closeTable(*this);
    ~RecordManager()
    {
    }

    bool isRecord(Rid r) const
    {
        auto p = _pm->getPage({r._fd, r._page});
        auto bm = BitMap(p->_data + sizeof(PageHeader), ceil(_th._slotsPerPage, BYTEINBITS));
        return bm.get(r._slot);
    }

    // read only
    const uint8_t *getRecordPointer(Rid r) const
    {
        return readSlot(r);
    }

    // get record copy
    std::unique_ptr<uint8_t[]> getRecord(Rid r) const
    {
        auto ptr = std::make_unique<uint8_t[]>(_th._recordSize);
        auto p = readSlot(r);
        memcpy(ptr.get(), p, _th._recordSize);
        return ptr;
    }

    Rid insertRecord(const void *data)
    {
        auto rid = getFreeSlot();
        writeSlot(rid, static_cast<const uint8_t *>(data));
        return rid;
    }

    void deleteRecord(Rid r)
    {
        deleteSlot(r);
    }

    void updateRecord(Rid r, const void *data)
    {
        writeSlot(r, static_cast<const uint8_t *>(data));
    }

    void flush(uint32_t pageNum, bool release = false)
    {
        if (pageNum == -1)
        {
            _pm->flushAllByFd(_fd, release);
        }
        else
        {
            if (_pm->isInCache({_fd, pageNum}))
            {
                auto p = _pm->getPage({_fd, pageNum});
                _pm->flush(p, release);
            }
        }
    }

    int getFd() const
    {
        return _fd;
    }

    PagedFile::PageManager *getPageManager() const
    {
        return _pm;
    }

    class Iterator
    {
      private:
        Rid _r;
        const RecordManager *_rm;

      public:
        Iterator() = delete;
        Iterator(const RecordManager *rm, Rid r) : _rm(rm), _r(r)
        {
        }
        Iterator &operator++()
        {
            auto p = _rm->getPageManager()->getPage({_r._fd, _r._page});
            auto bm = BitMap(p->_data + sizeof(PageHeader), ceil(_rm->_th._recordSize, BYTEINBITS));
            auto pos = bm.nextBit(_r._slot + 1, true);
            if (pos < _rm->_th._slotsPerPage)
            {
                _r._slot = pos;
            }
            else
            {
                if (_r._page == _rm->_th._existsPageNum) // bug fixed
                {
                    // end of all record
                    _r._slot = -1;
                }
                else
                {
                    auto npid = _r._page;
                    while (++npid <= _rm->_th._existsPageNum)
                    {
                        auto npage = _rm->getPageManager()->getPage({_r._fd, npid});
                        auto nbm = BitMap(npage->_data + sizeof(PageHeader), ceil(_rm->_th._recordSize, BYTEINBITS));
                        auto npos = nbm.nextBit(0, true);
                        if (npos < _rm->_th._existsPageNum)
                        {
                            _r._page = npid;
                            _r._slot = npos; // bug fixed
                            break;
                        }
                        else if (npid == _rm->_th._existsPageNum)
                        {
                            _r._page = npid;
                            _r._slot = -1;
                            break;
                        }
                    }
                }
            }
            return *this;
        }
        Iterator operator++(int) const
        {
            Iterator it(_rm, _r);
            return ++it;
        };
        bool operator==(Iterator other) const
        {
            return _r == other._r;
        }
        bool operator!=(Iterator other) const
        {
            return not(*this == other);
        }

        Rid getRid() const
        {
            return _r;
        }

        const uint8_t *operator*() const
        {
            return _rm->readSlot(_r);
        }
    };

    Iterator cbegin() const
    {
        if (_th._existsPageNum == 0)
            return cend();
        Rid r;
        r._fd = _fd;
        r._page = 1;
        r._slot = 0;

        auto it = Iterator(this, r);
        if (isRecord(r))
        {
            return it;
        }
        else
        {
            return ++it;
        }
    }
    Iterator cend() const
    {
        Rid r;
        r._fd = _fd;
        r._page = _th._existsPageNum;
        r._slot = -1;
        return Iterator(this, r);
    }
};

// sizeof(bitmap) = ceil(n/8) = (n + 8 - 1)/8
// sizeof(PageHeader) + sizeof(bitmap) + n * recordSize <= PAGESIZE
static uint32_t calSlotsPerPage(uint32_t recordSize)
{
    return (BYTEINBITS * (PAGESIZE - sizeof(PageHeader)) - (BYTEINBITS - 1)) / (BYTEINBITS * recordSize + 1);
}

/**
 * @brief RecordFileManager.
 * fixed length for a record now(2022/1/10).
 */
class RecordFileManager
{
  public:
    static RecordManager openTable(std::string_view path)
    {
        int fd = PagedFile::FileManager::openFile(path);
        auto pm = PagedFile::getPageManager();
        auto page = pm->getPage({fd, 0});
        TableHeader th;
        memcpy(&th, page->_data, sizeof(th));
        assert(th._recordSize != 0);
        // pm.getPage();
        return RecordManager(fd, pm, th);
    }
    static void closeTable(RecordManager &rm)
    {
        PagedFile::FileManager::closeFile(rm.getFd(), *(rm.getPageManager()));
    }
    static RecordManager creatTable(std::string_view path, uint32_t recordSize)
    {
        assert(recordSize <= MAXRECORDSIZE);
        PagedFile::FileManager::createFile(path);
        int fd = PagedFile::FileManager::openFile(path);
        auto pm = PagedFile::getPageManager();
        auto page = pm->getPage({fd, 0}); // first page just for header
        TableHeader th = {._recordSize = recordSize, ._existsPageNum = 0, ._nextPage = 1};
        th._slotsPerPage = calSlotsPerPage(recordSize);
        memcpy(page->_data, &th, sizeof(th));
        page->_dirty = true;
        return RecordManager(fd, pm, th);
    }
    static void deleteTable(std::string_view path)
    {
        PagedFile::FileManager::deleteFile(path);
    };
};
} // namespace RecordMgr

#endif // __SQLIGHT_RECORD__
