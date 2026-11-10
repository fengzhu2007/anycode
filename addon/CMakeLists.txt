cmake_minimum_required(VERSION 3.10)

# ---------------------------------------------------------------------------
# 第三方依赖全部由顶层 add_subdirectory(3party) 就地（in-tree）构建，
# 各 addon 子模块直接链接这些 CMake 目标，不再引用 ../3party/bin 下的预编译库。
# 这里统一解析一次，子目录通过父作用域变量直接使用。
# ---------------------------------------------------------------------------
if(TARGET CURL::libcurl)
    set(ADDON_CURL_LIB CURL::libcurl)
elseif(TARGET libcurl_shared)
    set(ADDON_CURL_LIB libcurl_shared)
elseif(TARGET libcurl_static)
    set(ADDON_CURL_LIB libcurl_static)
else()
    message(FATAL_ERROR "找不到 libcurl 目标：请确认顶层已 add_subdirectory(3party) 并参与构建")
endif()

add_subdirectory(FTP)
add_subdirectory(COS)
add_subdirectory(OSS)
add_subdirectory(SFTP)
add_subdirectory(S3)
add_subdirectory(python)
add_subdirectory(3DViewer)
