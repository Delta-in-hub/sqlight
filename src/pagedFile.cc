#include "pagedFile.h"

robin_hood::unordered_map<std::string, int> PagedFile::FileManager::_path2fd;
robin_hood::unordered_map<int, std::string> PagedFile::FileManager::_fd2path;

static std::vector<std::unique_ptr<PagedFile::PageManager>> vec;

PagedFile::PageManager *PagedFile::getPageManager()
{
    if (vec.empty())
    {
        auto p = std::make_unique<PagedFile::PageManager>();
        assert(p.get() != nullptr);
        vec.emplace_back(move(p));
    }
    return vec.front().get();
}