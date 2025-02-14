cmake_minimum_required(VERSION 3.13)

project(tree-sitter-embedded-template
        VERSION "0.23.2"
        DESCRIPTION "Embedded Template (ERB, EJS) grammar for tree-sitter"
        HOMEPAGE_URL "https://github.com/tree-sitter/tree-sitter-embedded-template"
        LANGUAGES C)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
option(TREE_SITTER_REUSE_ALLOCATOR "Reuse the library allocator" OFF)

add_library(tree-sitter-embedded-template STATIC parser.c)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/scanner.c)
  target_sources(tree-sitter-embedded-template PRIVATE scanner.c)
endif()
target_include_directories(tree-sitter-embedded-template PRIVATE src)

target_compile_definitions(tree-sitter-embedded-template PRIVATE
                           $<$<BOOL:${TREE_SITTER_REUSE_ALLOCATOR}>:TREE_SITTER_REUSE_ALLOCATOR>
                           $<$<CONFIG:Debug>:TREE_SITTER_DEBUG>)

set_target_properties(tree-sitter-embedded-template
                      PROPERTIES
                      C_STANDARD 11
                      POSITION_INDEPENDENT_CODE ON
                      SOVERSION "${TREE_SITTER_ABI_VERSION}.${PROJECT_VERSION_MAJOR}"
                      DEFINE_SYMBOL "")

