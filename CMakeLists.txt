cmake_minimum_required(VERSION 3.0.0)
project(sqlight VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)

# CTest related
enable_testing()
add_subdirectory(test)