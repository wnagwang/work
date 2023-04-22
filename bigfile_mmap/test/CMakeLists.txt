cmake_minimum_required(VERSION 3.0)
project(bigfile_mmap)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated  -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

include_directories(.)
include_directories(../include)
include_directories(../src)
include_directories(../test)
link_directories(../lib)

file(GLOB_RECURSE LIB_SRC
        ../src/*.cpp    
        )


add_executable(file_op_test file_op_test.cpp   ${LIB_SRC})
add_executable(mmap_file_test mmap_file_test.cpp  ${LIB_SRC})
add_executable(mmap_file_op_test mmap_file_op_test.cpp  ${LIB_SRC})
add_executable(block_init_test block_init_test.cpp  ${LIB_SRC})
add_executable(block_write_test block_write_test.cpp  ${LIB_SRC})
add_executable(block_stat block_stat.cpp  ${LIB_SRC})
add_executable(block_read_test block_read_test.cpp  ${LIB_SRC})
add_executable(block_delete_test block_delete_test.cpp  ${LIB_SRC})
