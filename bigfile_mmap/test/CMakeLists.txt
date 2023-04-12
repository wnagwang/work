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
#add_library(mmap STATIC ${LIB_SRC})

#add_subdirectory(../src/ src_binary_dir)


add_executable(file_op_test file_op_test.cpp   ${LIB_SRC})
add_executable(mmap_file_test mmap_file_test.cpp  ${LIB_SRC})
add_executable(mmap_file_op_test mmap_file_op_test.cpp  ${LIB_SRC})