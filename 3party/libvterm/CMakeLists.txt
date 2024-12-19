cmake_minimum_required(VERSION 3.14)

project(libvterm)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(SYSTEM "./include")

add_library(libvterm STATIC
    src/encoding.c
    src/fullwidth.inc
    src/keyboard.c
    src/mouse.c
    src/parser.c
    src/pen.c
    src/rect.h
    src/screen.c
    src/state.c
    src/unicode.c
    src/utf8.h
    src/vterm.c
    src/vterm_internal.h
)

if(LINUX)
    target_compile_options(libvterm PRIVATE -fPIC)
endif()
