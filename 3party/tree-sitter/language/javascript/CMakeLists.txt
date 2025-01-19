cmake_minimum_required(VERSION 3.13)

project(tree-sitter-javascript
        VERSION "0.23.1"
        DESCRIPTION "JavaScript grammar for tree-sitter"
        HOMEPAGE_URL "https://github.com/tree-sitter/tree-sitter-javascript"
        LANGUAGES C)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
option(TREE_SITTER_REUSE_ALLOCATOR "Reuse the library allocator" OFF)

set(TREE_SITTER_ABI_VERSION 14 CACHE STRING "Tree-sitter ABI version")
if(NOT ${TREE_SITTER_ABI_VERSION} MATCHES "^[0-9]+$")
    unset(TREE_SITTER_ABI_VERSION CACHE)
    message(FATAL_ERROR "TREE_SITTER_ABI_VERSION must be an integer")
endif()



add_library(tree-sitter-javascript STATIC parser.c)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/scanner.c)
  target_sources(tree-sitter-javascript PRIVATE scanner.c)
endif()
target_include_directories(tree-sitter-javascript PRIVATE src)

target_compile_definitions(tree-sitter-javascript PRIVATE
                           $<$<BOOL:${TREE_SITTER_REUSE_ALLOCATOR}>:TREE_SITTER_REUSE_ALLOCATOR>
                           $<$<CONFIG:Debug>:TREE_SITTER_DEBUG>)

set_target_properties(tree-sitter-javascript
                      PROPERTIES
                      C_STANDARD 11
                      POSITION_INDEPENDENT_CODE ON
                      SOVERSION "${TREE_SITTER_ABI_VERSION}.${PROJECT_VERSION_MAJOR}"
                      DEFINE_SYMBOL "")

