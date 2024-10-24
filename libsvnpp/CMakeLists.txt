cmake_minimum_required(VERSION 3.14)

project(libsvnpp)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -MD -GL")


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)


#3party\bin\x64\include
include_directories(SYSTEM "./include")
include_directories(SYSTEM "../3party/bin/x64/include")
include_directories(SYSTEM "../3party/bin/x64/include")
include_directories(SYSTEM "../3party/subversion/subversion/include")

find_library(APR_LIB libapr-1 PATHS "../3party/bin/x64/lib")
find_library(APR_UTIL_LIB libaprutil-1 PATHS "../3party/bin/x64/lib")
find_library(EXPAT_LIB libexpat PATHS "../3party/bin/x64/lib")
find_library(Z_LIB zlibstatic PATHS "../3party/bin/x64/lib")
find_library(CRYPTO_LIB libcrypto PATHS "../3party/bin/x64/lib")
find_library(SSL_LIB libssl PATHS "../3party/bin/x64/lib")
find_library(SVN_SUBR_LIB svn_subr-1 PATHS "../3party/subversion/Release/subversion/libsvn_subr")
find_library(SVN_DELTA_LIB svn_delta-1 PATHS "../3party/subversion/Release/subversion/libsvn_delta")
find_library(SVN_CLIENT_LIB svn_client-1 PATHS "../3party/subversion/Release/subversion/libsvn_client")
find_library(SVN_RA_LIB svn_ra-1 PATHS "../3party/subversion/Release/subversion/libsvn_ra")

find_library(RA_SVN libsvn_ra_svn-1 PATHS "../3party/subversion/Release/subversion/libsvn_ra_svn")
find_library(RA_LOCAL libsvn_ra_local-1 PATHS "../3party/subversion/Release/subversion/libsvn_ra_local")

find_library(SVN_REPOS_LIB svn_repos-1 PATHS "../3party/subversion/Release/subversion/libsvn_repos")
find_library(SVN_DIFF_LIB svn_diff-1 PATHS "../3party/subversion/Release/subversion/libsvn_diff")
find_library(SVN_FS_LIB svn_fs-1 PATHS "../3party/subversion/Release/subversion/libsvn_fs")
find_library(SVN_FS_FS_LIB svn_fs_fs-1 PATHS "../3party/subversion/Release/subversion/libsvn_fs_fs")
find_library(SVN_FS_UTIL_LIB svn_fs_util-1 PATHS "../3party/subversion/Release/subversion/libsvn_fs_util")
find_library(SVN_FS_X_LIB svn_fs_x-1 PATHS "../3party/subversion/Release/subversion/libsvn_fs_x")
find_library(SVN_WC_LIB svn_wc-1 PATHS "../3party/subversion/Release/subversion/libsvn_wc")





add_library(libsvnpp SHARED
        src/svnpp_client.cpp
		src/client/diff_summary.cpp
		src/client/info.cpp
		src/client/log_entry.cpp
)

target_link_libraries(libsvnpp PRIVATE Qt${QT_VERSION_MAJOR}::Core ${APR_LIB} ${APR_UTIL_LIB} ${EXPAT_LIB} ${Z_LIB} ${CRYPTO_LIB} ${SSL_LIB} ${RA_SVN} ${RA_LOCAL} ${SVN_REPOS_LIB} ${SVN_SUBR_LIB} ${SVN_DELTA_LIB} ${SVN_CLIENT_LIB} ${SVN_RA_LIB} ${SVN_DIFF_LIB} ${SVN_FS_LIB} ${SVN_FS_FS_LIB} ${SVN_FS_UTIL_LIB} ${SVN_FS_X_LIB} ${SVN_WC_LIB}  ws2_32 rpcrt4 mswsock  advapi32   crypt32 version)

target_compile_definitions(libsvnpp PRIVATE SVNPP_LIBRARY)

message("The value of MY_VARIABLE is: ${CMAKE_BINARY_DIR}")

message("Project name: ${PROJECT_NAME}")


add_custom_command(TARGET libsvnpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}.dll
        ${CMAKE_BINARY_DIR}/src/${PROJECT_NAME}.dll
    COMMENT "Copying file to binary directory"
)


