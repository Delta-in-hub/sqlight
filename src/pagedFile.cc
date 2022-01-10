#include "pagedFile.h"

robin_hood::unordered_map<std::string, int> PagedFile::FileManager::_path2fd;
robin_hood::unordered_map<int, std::string> PagedFile::FileManager::_fd2path;

static std::vector<PagedFile::PageManager *> vec;

PagedFile::PageManager &PagedFile::getPageManager()
{
    if (vec.empty())
    {
        auto p = new PageManager;
        assert(p);
        vec.emplace_back(p);
    }
    return *vec.front();
}

void PagedFile::destoryAllPageManager()
{
    for (auto &&i : vec)
    {
        delete i;
    }
    vec.clear();
}