cmake_minimum_required(VERSION 3.0)
project(file)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

include_directories(.)
include_directories(/include)
include_directories(/src)
include_directories(/test)
link_directories(/lib)

file(GLOB_RECURSE LIB_SRC
        /src/*.cpp    
        )
add_library(mmap STATIC ${LIB_SRC})

install(TARGETS mmap demo
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)


add_executable(file_op_test /test/file_op_test.cpp  ${LIB_SRC})
add_executable(mmap_file_test /test/mmap_file_test.cpp ${LIB_SRC})
add_executable(mmap_file_op_test /test/mmap_file_op_test.cpp ${LIB_SRC})