project(SFTP)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql LinguistTools)

# curl 头文件直接取自 3party 源码目录，库由顶层 3party 的 in-tree 目标提供（ADDON_CURL_LIB）
# SFTP 走 curl 内置的 libssh2 后端，无需单独链接 libssh2
include_directories(SYSTEM "../../App")
include_directories(SYSTEM "../../3party/curl/include")


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DQ_DEBUG)
endif()
set(FORMS
    sftp_form_dir_setting.ui
    sftp_form_general.ui
)

# Qt5/Qt6 兼容：Qt6 未提供 qt5_wrap_ui
if(QT_VERSION_MAJOR EQUAL 6)
    qt6_wrap_ui(FORMS_HEADERS ${FORMS})
else()
    qt5_wrap_ui(FORMS_HEADERS ${FORMS})
endif()


add_library(SFTP SHARED
    ${FORMS_HEADERS}
    ${QM_FILES}
    sftp_global.h export.h export.cpp
    sftp_form_dir_setting.h sftp_form_dir_setting.cpp
    sftp_form_general.h sftp_form_general.cpp
    sftp_parse.h sftp_parse.cpp
    sftp_response.h sftp_response.cpp
    sftp_setting_key.h sftp_setting.h sftp_setting.cpp
    sftp.h sftp.cpp
)

target_link_libraries(SFTP PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql App ${ADDON_CURL_LIB})

target_compile_definitions(SFTP PRIVATE SFTP_LIBRARY)

if(WIN32)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy   ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll  ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll
    #COMMAND ${CMAKE_COMMAND} -E copy   ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/language.zh_CN.qm    ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/language.zh_CN.qm
    COMMENT "Copying file to binary directory"
)
elseif()

endif()

