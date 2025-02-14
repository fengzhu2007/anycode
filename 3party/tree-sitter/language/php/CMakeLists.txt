cmake_minimum_required(VERSION 3.13)

project(tree-sitter-php
        VERSION "0.23.11"
		DESCRIPTION "PHP & PHP-Only grammars for tree-sitter"
        HOMEPAGE_URL "https://github.com/tree-sitter/tree-sitter-php"
        LANGUAGES C)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(TREE_SITTER_REUSE_ALLOCATOR "Reuse the library allocator" OFF)

set(TREE_SITTER_ABI_VERSION 14 CACHE STRING "Tree-sitter ABI version")
if(NOT ${TREE_SITTER_ABI_VERSION} MATCHES "^[0-9]+$")
    unset(TREE_SITTER_ABI_VERSION CACHE)
    message(FATAL_ERROR "TREE_SITTER_ABI_VERSION must be an integer")
endif()


macro(add_parser name)

    add_library(tree-sitter-${name} STATIC
                "${CMAKE_CURRENT_SOURCE_DIR}/parser.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/scanner.c")
    target_include_directories(tree-sitter-${name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

    target_compile_definitions(tree-sitter-${name} PRIVATE
                               $<$<BOOL:${TREE_SITTER_REUSE_ALLOCATOR}>:TREE_SITTER_REUSE_ALLOCATOR>
                               $<$<CONFIG:Debug>:TREE_SITTER_DEBUG>)

    set_target_properties(tree-sitter-${name}
                          PROPERTIES
                          C_STANDARD 11
                          POSITION_INDEPENDENT_CODE ON
                          SOVERSION "${TREE_SITTER_ABI_VERSION}.${PROJECT_VERSION_MAJOR}"
                          DEFINE_SYMBOL "")

endmacro()

add_subdirectory(php tree-sitter-php)
add_subdirectory(php_only tree-sitter-php_only)
