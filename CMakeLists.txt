cmake_minimum_required(VERSION 3.0.0)
project(sqlight VERSION 0.0.1)
message(${PROJECT_NAME})

add_executable(sqlightcli main.cpp)
target_compile_features(sqlightcli PRIVATE cxx_std_17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")


find_package(fmt)
# find_package(Botan2)

target_link_libraries(sqlightcli fmt::fmt pthread)