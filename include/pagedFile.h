#if !defined(__SQLIGHT_PAGEDFILE__)
#define __SQLIGHT_PAGEDFILE__

#include <cassert>
#include <cstdlib>
#include <fcntl.h>
#include <list>
#include <memory>
#include <queue>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>

namespace PagedFile
{

/**
 * @brief handle file through os.
 * create / open / delete file
 */
class FileManager
{
  private:
    static std::unordered_map<std::string, int> _path2fd;
    static std::unordered_map<int, std::string> _fd2path;

  public:
    int getFd(std::string_view path)
    {
        return _path2fd[path.data()];
    }
    std::string getFullPath(int fd)
    {
        return _fd2path[fd];
    }
    /**
     * @brief check a path is a normal file.
     * if ture return its full path , else return empty.
     * @param path
     * @return std::string
     */
    static std::string isFile(std::string_view path)
    {
        struct stat st;
        if (stat(path.data(), &st) == 0 and S_ISREG(st.st_mode))
        {
            auto fullpath = realpath(path.data(), NULL);
            assert(fullpath);
            std::string str(fullpath);
            free(fullpath);
            return str;
        }
        return "";
    }
    static int openFile(std::string_view path)
    {
        auto fpath = isFile(path);
        assert(not fpath.empty());
        auto pos = _path2fd.find(fpath);
        if (pos != _path2fd.end())
        {
            return pos->second;
        }
        int fd = open(path.data(), O_RDWR | O_DIRECT | O_ASYNC);
        assert(fd != -1);
        _path2fd[fpath] = fd;
        _fd2path[fd] = fpath;
        return fd;
    }
    static void closeFile(int fd)
    {
        auto pos = _fd2path.find(fd);
        assert(pos != _fd2path.end());
        _path2fd.erase(pos->second);
        _fd2path.erase(pos);
        // Something
        // ...
        close(fd);
    }
    static void createFile(std::string_view path)
    {
        auto fpath = isFile(path);
        assert(fpath.empty());
        int fd = open(path.data(), O_CREAT, S_IRUSR | S_IWUSR);
        assert(fd != -1);
        close(fd);
    }
    static void deleteFile(std::string_view path)
    {
        auto fpath = isFile(path);
        assert(fpath.size());
        assert(_path2fd.count(fpath) == 0);
        unlink(path.data());
    }
};

constexpr unsigned PAGESIZE = 4096; // A page is 4096 bytes.

constexpr unsigned CACHESIZE = 4096 * 2; // Cache could contains CACHESIZE pages for maximum.

struct Pid
{
    int fd;
    int pageNum;
    bool operator=(const Pid &other) const
    {
        return std::tie(fd, pageNum) == std::tie(other.fd, other.pageNum);
    }
};

struct PidHash
{
    size_t operator()(const Pid &p) const
    {
        auto h1 = std::hash<int>{}(p.fd);

        // https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
        h1 ^= std::hash<int>{}(p.pageNum) + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
        return h1;
    }
};

struct Page
{
    Pid _id;
    uint8_t *_data;
    bool _dirty;
};

/**
 * @brief cache for page
 *
 */
class PageManager
{
  private:
    std::unique_ptr<uint8_t, decltype(&std::free)> _cache{nullptr, &std::free};
    Page _page[CACHESIZE];

    std::list<Page *> _usedPage;
    std::unordered_map<Pid, decltype(_usedPage.begin()), PidHash> _hashm;

    std::priority_queue<Page *, std::vector<Page *>, std::greater<Page *>> _unusedPage;

    /**
     * @brief write back a page to disk
     *
     * @param p
     */
    void writeToDisk(Page *p)
    {
        if (p->_dirty)
        {
            lseek(p->_id.fd, p->_id.pageNum * PAGESIZE, SEEK_SET);
            auto wsize = write(p->_id.fd, p->_data, PAGESIZE);
            assert(wsize == PAGESIZE);
            p->_dirty = false;
        }
    }

    void readFromDisk(Page *p)
    {
        lseek(p->_id.fd, p->_id.pageNum * PAGESIZE, SEEK_SET);
        auto nr = read(p->_id.fd, p->_data, PAGESIZE);
        assert(nr == PAGESIZE);
    }

  public:
    PageManager()
    {
        auto p = aligned_alloc(4096, PAGESIZE * CACHESIZE); // for direct_io
        assert(p);
        _cache.reset(static_cast<uint8_t *>(p));
        for (int i = 0; i < CACHESIZE; i++)
        {
            _page[i]._data = _cache.get() + i * PAGESIZE;
            _page[i]._dirty = false;
            _unusedPage.emplace(&(_page[i]));
        }
    }
    Page *getPage(const Pid &p, bool fromDisk = false)
    {
        Page *ans = nullptr;
        auto pos = _hashm.find(p);
        if (pos != _hashm.end())
        {
            ans = *(pos->second);
        }
        else
        {
            if (_unusedPage.empty())
            {
                flush(_usedPage.back());
            }

            assert(not _unusedPage.empty());
            ans = _unusedPage.top();
            _unusedPage.pop();

            ans->_dirty = false;
            ans->_id = p;
            _usedPage.push_front(ans);
            _hashm[p] = _usedPage.begin();
            if (fromDisk)
            {
                readFromDisk(ans);
            }
        }
        return ans;
    }

    /**
     * @brief write back a page to disk ,maybe remove it from cache
     *
     * @param del  True for remove it from cache
     * @param p
     */
    void flush(Page *p, bool del = true)
    {
        writeToDisk(p);
        if (del)
        {
            auto pos = _hashm.find(p->_id);
            _usedPage.erase(pos->second);
            _unusedPage.emplace(p);
            _hashm.erase(pos);
        }
    }

    void flushAll(bool del = true)
    {
        for (auto &&i : _usedPage)
        {
            flush(i, del);
        }
    }
};

} // namespace PagedFile

#endif // __SQLIGHT_PAGEDFILE__
