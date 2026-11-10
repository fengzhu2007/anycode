if(WIN32)
    add_subdirectory(winpty)
endif()
add_subdirectory(libvterm)
add_subdirectory(libptyqt)

add_subdirectory(tree-sitter)


set(CMAKE_SKIP_INSTALL_RULES ON)




cmake_policy(SET CMP0077 NEW)
set(CURL_USE_LIBPSL OFF)


set(BUILD_SHARED_LIBS ON CACHE BOOL "Build as shared library" FORCE)

set(BUILD_TESTING OFF CACHE BOOL "Disable curl tests" FORCE)
set(ENABLE_PROGRAMS OFF CACHE BOOL "Build mbed TLS programs" FORCE)
set(ENABLE_TESTING OFF CACHE BOOL "Build mbed TLS tests" FORCE)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# 静态 zlib/libssh2 会被链进共享 libcurl，Linux/macOS 下需要 PIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(GEN_FILES ON CACHE BOOL "Generate missing source files" FORCE)
set(MBEDTLS_GEN_FILES ON CACHE BOOL "Generate missing source files" FORCE)

# 显式关闭 mbedtls 的安装规则
set(MBEDTLS_ENABLE_INSTALL OFF CACHE BOOL "Disable mbedtls install" FORCE) # 新版 mbedtls
set(USE_PKCS11_HELPER_LIBRARY OFF CACHE BOOL "" FORCE)

add_subdirectory(mbedtls)



set(ZLIB_BUILD_SHARED OFF CACHE BOOL "Build zlib shared library" FORCE)
set(ZLIB_BUILD_STATIC ON CACHE BOOL "Build zlib static library" FORCE)
set(ZLIB_INSTALL OFF CACHE BOOL "Disable zlib install" FORCE)

add_subdirectory(zlib)

# zconf.h 是 zlib 构建时 configure_file 生成到 binary dir 的。
# zlib 的 CMake 默认只把源码目录暴露给使用者（$<BUILD_INTERFACE:源码目录>），
# 外部目标 include zlib.h 时会因为找不到 zconf.h 报 C1083。这里补上 binary dir。
if(TARGET zlibstatic)
    target_include_directories(zlibstatic INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/zlib>)
endif()
if(TARGET zlib)
    target_include_directories(zlib INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/zlib>)
endif()




set(CURL_USE_LIBPSL OFF)
set(CURL_USE_MBEDTLS ON CACHE BOOL "Use mbedtls for TLS" FORCE)
set(CURL_USE_OPENSSL OFF CACHE BOOL "Disable OpenSSL" FORCE)
set(CURL_ZLIB ON CACHE BOOL "Enable zlib support" FORCE)

set(MBEDTLS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/mbedtls/include" CACHE PATH "Mbed TLS include directory" FORCE)
set(MBEDTLS_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/mbedtls/include" CACHE PATH "Mbed TLS include directory" FORCE)
set(MBEDTLS_LIBRARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/mbedtls/library" CACHE PATH "Mbed TLS library directory" FORCE)

set(MBEDTLS_LIBRARY "mbedtls" CACHE STRING "Mbed TLS library" FORCE)
set(MBEDX509_LIBRARY "mbedx509" CACHE STRING "Mbed X509 library" FORCE)
set(MBEDCRYPTO_LIBRARY "mbedcrypto" CACHE STRING "Mbed Crypto library" FORCE)

# ---------------- zlib ----------------
# ---------------- zlib ----------------
# 同时给出源码目录（zlib.h 在此）和 binary 目录（zconf.h 在此），
# 供 curl 的 FindZLIB 使用；也给到 ZLIB_INCLUDE_DIRS 以防探测脚本用复数名。
set(ZLIB_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/zlib;${CMAKE_BINARY_DIR}/3party/zlib"
    CACHE STRING "zlib include directories" FORCE)
set(ZLIB_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/zlib;${CMAKE_BINARY_DIR}/3party/zlib"
    CACHE STRING "zlib include directories" FORCE)
set(ZLIB_LIBRARY "zlibstatic" CACHE STRING "zlib static library (in-tree target)" FORCE)
if(TARGET zlibstatic AND NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()


# ---------------- libssh2（curl 的 SCP/SFTP 后端） ----------------
set(BUILD_EXAMPLES OFF CACHE BOOL "Disable libssh2 examples" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Disable libssh2 testing" FORCE)
set(CRYPTO_BACKEND "mbedTLS" CACHE STRING "libssh2 cryptography backend" FORCE)

set(BUILD_SHARED_LIBS OFF)
set(BUILD_STATIC_LIBS ON)
macro(export)
endmacro()


add_subdirectory(libssh2)


macro(export)
    _export(${ARGN})
endmacro()

set(BUILD_SHARED_LIBS ON)
set(BUILD_STATIC_LIBS OFF)

# curl 的 libssh2 支持由 find_package(LibSSH2) 探测，指向 in-tree 构建的 libssh2_static
set(LIBSSH2_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libssh2/include" CACHE PATH "libssh2 include directory" FORCE)
set(LIBSSH2_LIBRARY "libssh2_static" CACHE STRING "libssh2 static library (in-tree target)" FORCE)




# curl 扩展依赖：zlib（传输压缩）+ libssh2（SCP/SFTP 协议）
set(CURL_ZLIB ON CACHE BOOL "Enable zlib support" FORCE)
set(CURL_USE_LIBSSH2 ON CACHE BOOL "Enable libssh2 support" FORCE)
set(CURL_DISABLE_INSTALL ON CACHE BOOL "Disable curl install" FORCE)

add_subdirectory(curl)


set(BUILD_TESTS OFF CACHE BOOL "Disable libgit2 tests" FORCE)
set(BUILD_CLI OFF CACHE BOOL "Disable libgit2 CLI" FORCE)
set(STATIC_CRT OFF CACHE BOOL "Use dynamic CRT (/MD and /MDd) to match Qt" FORCE)
set(USE_BUNDLED_ZLIB OFF CACHE BOOL "Use our in-tree zlib" FORCE)
add_subdirectory(libgit2)

# 兜底：确保 libcurl 链接到 in-tree 的 zlib/libssh2（同时建立构建顺序依赖）
# libcurl_* 是在 curl 子目录里创建的，跨目录调用 target_link_libraries 需要 CMP0079 NEW
# （顶层 cmake_minimum_required 为 3.10，策略默认 OLD，会报 "not built in this directory"）
if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif()
foreach(_curl_target libcurl_shared libcurl_static)
    if(TARGET ${_curl_target})
        if(TARGET zlibstatic)
            target_link_libraries(${_curl_target} PRIVATE zlibstatic)
        endif()
        if(TARGET libssh2_static)
            target_link_libraries(${_curl_target} PRIVATE libssh2_static)
        endif()
    endif()
endforeach()












