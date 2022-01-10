#if !defined(__SQLIGHT_RECORD__)
#define __SQLIGHT_RECORD__

#include "pagedFile.h"

// Record Manager
namespace RecordMgr
{

struct TableHeader
{
    uint32_t _recordSize;     // a record size in byte
    uint32_t _existsPageNum;  // existed page number in the file
    uint32_t _recordsPerPage; // record number of one page could have
    uint32_t _nextPage;       // next page waiting for be used
};

struct PageHeader
{
    uint32_t _existsRecordNum;
    uint32_t _nextSlot;
};

class RecordFileManager
{
  public:
    static RecordManager openTable(std::string_view path)
    {
        int fd = PagedFile::FileManager::openFile(path);
        auto &pm = PagedFile::getPageManager();
        auto page = pm.getPage({fd, 0});
        TableHeader th;
        memcpy(&th, page->_data, sizeof(th));
        assert(th._recordSize != 0);
        // pm.getPage();
        // return RecordManager(fd, );
    }
    static RecordManager creatTable(std::string_view path){};
    static void deleteTable(std::string_view path)
    {
        PagedFile::FileManager::deleteFile(path);
    };
};

class Rid
{
    int _fd;
    int _page;
    int _slot;
};

/**
 * @brief Record manager of a table
 *
 */
class RecordManager
{
  private:
    int _fd;
    PagedFile::PageManager &_pm;
    size_t _recordSize;
    TableHeader _tableHeader;

  public:
    /**
     * @brief Construct a new Record Manager object
     *
     * @param path
     * @param pm
     */
    RecordManager(int fd, PagedFile::PageManager &pm, size_t rsize) : _pm(pm), _fd(fd), _recordSize(rsize)
    {
        ;
    }

    bool isRecord(Rid r)
    {
        ;
    }

    std::unique_ptr<uint8_t[]> getRecord(Rid r)
    {
        auto ptr = std::make_unique<uint8_t[]>(123);
        return ptr;
    }

    Rid insertRecord()
    {
        ;
    }

    void deleteRecord(Rid r)
    {
        ;
    }

    void updateRecord(Rid r, uint8_t *data)
    {
        ;
    }
};

class RecordIterator
{
};

} // namespace RecordMgr

#endif // __SQLIGHT_RECORD__
