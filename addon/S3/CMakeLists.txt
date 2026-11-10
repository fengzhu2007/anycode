cmake_minimum_required(VERSION 3.14)

project(S3)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql LinguistTools)

# curl 头文件直接取自 3party 源码目录，库由顶层 3party 的 in-tree 目标提供（ADDON_CURL_LIB）
# 签名用的 HMAC 已改用 Qt 内置的 QMessageAuthenticationCode，不再依赖 openssl
include_directories(SYSTEM "../../App")
include_directories(SYSTEM "../../3party/curl/include")


set(FORMS
    s3_form_dir_setting.ui
    s3_form_general.ui

)

# Qt5/Qt6 兼容：Qt6 未提供 qt5_wrap_ui
if(QT_VERSION_MAJOR EQUAL 6)
    qt6_wrap_ui(FORMS_HEADERS ${FORMS})
else()
    qt5_wrap_ui(FORMS_HEADERS ${FORMS})
endif()



add_library(S3 SHARED
    ${FORMS_HEADERS}

    s3_global.h
    export.h export.cpp
    s3_form_dir_setting.h s3_form_dir_setting.cpp
    s3_form_general.h s3_form_general.cpp
    s3_parse.h s3_parse.cpp
    s3_response.h s3_response.cpp
    s3_setting.h s3_setting_key.h s3_setting.cpp
    s3_list_object_response.h s3_list_object_response.cpp
    s3.h s3.cpp
)

target_link_libraries(S3 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql App ${ADDON_CURL_LIB})

target_compile_definitions(S3 PRIVATE S3_LIBRARY)


if(WIN32)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy  ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll
    COMMENT "Copying file to binary directory"
)

elseif()

endif()
