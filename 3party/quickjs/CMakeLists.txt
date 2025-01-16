cmake_minimum_required(VERSION 3.14)

project(quickjs)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(SYSTEM "./include")

add_library(quickjs STATIC
    cutils.c libbf.c libregexp.c libunicode.c quickjs.c cutils.h libbf.h libregexp.h libregexp-opcode.h libunicode.h libunicode-table.h list.h quickjs.h quickjs-atom.h quickjs-opcode.h
)

if(LINUX)
    target_compile_options(quickjs PRIVATE -fPIC)
endif()
