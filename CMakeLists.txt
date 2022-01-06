cmake_minimum_required(VERSION 3.0.0)
project(sqlight VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_subdirectory(extern/fmt)

if(WIN32)
  set(thread "")
else()
  set(thread pthread)
endif(WIN32)


add_executable(sqlightcli main.cc src/bitwise.cc src/utf8.cc)
target_link_libraries(sqlightcli fmt::fmt ${thread})


enable_testing()
add_subdirectory(extern/gtest)
include(GoogleTest)

add_executable(
  sqlighttest
  test/main.cc
  src/bitwise.cc
)
target_link_libraries(
  sqlighttest
  gtest_main
  fmt::fmt
  ${thread}
)