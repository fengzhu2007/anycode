cmake_minimum_required(VERSION 3.14)

project({TPL})

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql LinguistTools)

# curl 头文件直接取自 3party 源码目录，库由顶层 3party 的 in-tree 目标提供（ADDON_CURL_LIB）
# 签名用的 HMAC 请使用 Qt 内置的 QMessageAuthenticationCode，不要引入 openssl
include_directories(SYSTEM "../../App")
include_directories(SYSTEM "../../3party/curl/include")


set(FORMS
    {tpl}_form_dir_setting.ui
    {tpl}_form_general.ui

)

# Qt5/Qt6 兼容：Qt6 未提供 qt5_wrap_ui
if(QT_VERSION_MAJOR EQUAL 6)
    qt6_wrap_ui(FORMS_HEADERS ${FORMS})
else()
    qt5_wrap_ui(FORMS_HEADERS ${FORMS})
endif()



add_library({TPL} SHARED
    ${FORMS_HEADERS}

    {tpl}_global.h
    export.h export.cpp
    {tpl}_form_dir_setting.h {tpl}_form_dir_setting.cpp
    {tpl}_form_general.h {tpl}_form_general.cpp
    {tpl}_parse.h {tpl}_parse.cpp
    {tpl}_response.h {tpl}_response.cpp
    {tpl}_setting.h {tpl}_setting_key.h {tpl}_setting.cpp
    {tpl}.h {tpl}.cpp
)

target_link_libraries({TPL} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql App ${ADDON_CURL_LIB})

target_compile_definitions({TPL} PRIVATE {TPL}_LIBRARY)


if(WIN32)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy  ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll
    #COMMAND ${CMAKE_COMMAND} -E copy  ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/language.zh_CN.qm  ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/language.zh_CN.qm
    COMMENT "Copying file to binary directory"
)

elseif()

endif()