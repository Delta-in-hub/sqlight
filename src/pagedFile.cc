#include "pagedFile.h"

robin_hood::unordered_map<std::string, int> PagedFile::FileManager::_path2fd;
robin_hood::unordered_map<int, std::string> PagedFile::FileManager::_fd2path;