#include "pagedFile.h"

std::unordered_map<std::string, int> PagedFile::FileManager::_path2fd;
std::unordered_map<int, std::string> PagedFile::FileManager::_fd2path;