cmake_minimum_required(VERSION 3.14)

project({TPL})

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql LinguistTools)

find_library(CRYPTO_LIB libcrypto PATHS "../3party/bin/x64/lib")
find_library(SSL_LIB libssl PATHS "../3party/bin/x64/lib")
find_library(CURL_LIB libcurl_imp PATHS "../3party/bin/x64/lib")

include_directories(SYSTEM "../../App")
include_directories(SYSTEM "../../3party/bin/x64/include")


set(FORMS
    chmod_dialog.ui
    {tpl}_form_dir_setting.ui
    {tpl}_form_general.ui
    {tpl}_panel.ui

)

qt5_wrap_ui(FORMS_HEADERS ${FORMS})



add_library({TPL} SHARED
    ${FORMS_HEADERS}

    {tpl}_global.h
    chmod_dialog.h chmod_dialog.cpp
    export.h export.cpp
    {tpl}_form_dir_setting.h {tpl}_form_dir_setting.cpp
    {tpl}_form_general.h {tpl}_form_general.cpp
    {tpl}_panel.h {tpl}_panel.cpp
    {tpl}_parse.h {tpl}_parse.cpp
    {tpl}_response.h {tpl}_response.cpp
    {tpl}_setting.h {tpl}_setting_key.h {tpl}_setting.cpp
    {tpl}_worker.h {tpl}_worker.cpp
    {tpl}.h {tpl}.cpp
)

target_link_libraries({TPL} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql App ${CRYPTO_LIB} ${SSL_LIB}  ${CURL_LIB})

target_compile_definitions({TPL} PRIVATE {TPL}_LIBRARY)



add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy  ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll
    #COMMAND ${CMAKE_COMMAND} -E copy  ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/language.zh_CN.qm  ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/language.zh_CN.qm
    COMMENT "Copying file to binary directory"
)

