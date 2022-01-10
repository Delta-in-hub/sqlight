cmake_minimum_required(VERSION 3.0.0)
project(sqlight VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories("include")

file(GLOB SOURCEFILE "src/*.cc")
file(GLOB HEADERFILE "include/*.h")

add_subdirectory(extern/fmt)
add_subdirectory(extern/robin_hood)

if(WIN32)
  set(thread "")
else()
  set(thread pthread)
endif(WIN32)


add_executable(sqlightcli main.cc ${SOURCEFILE} ${HEADERFILE})
target_link_libraries(sqlightcli fmt::fmt ${thread} robin_hood::robin_hood)


enable_testing()
add_subdirectory(extern/gtest)
include(GoogleTest)

add_executable(
  sqlighttest
  test/main.cc
  ${SOURCEFILE} 
  ${HEADERFILE}
)
target_link_libraries(
  sqlighttest
  gtest_main
  fmt::fmt
  ${thread}
  robin_hood::robin_hood
)