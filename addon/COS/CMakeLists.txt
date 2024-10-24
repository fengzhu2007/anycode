cmake_minimum_required(VERSION 3.14)

project(COS)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql LinguistTools )
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql LinguistTools )

find_library(CRYPTO_LIB libcrypto PATHS "../3party/bin/x64/lib")
find_library(SSL_LIB libssl PATHS "../3party/bin/x64/lib")
find_library(CURL_LIB libcurl_imp PATHS "../3party/bin/x64/lib")
#find_library(SSH2_LIB libssh2 PATHS "../3party/bin/x64/lib")

include_directories(SYSTEM "../../App")
include_directories(SYSTEM "../../3party/bin/x64/include")
include_directories(SYSTEM "../../3party/openssl/crypto")



 set(FORMS
        cos_form_dir_setting.ui
        cos_form_general.ui
        cos_panel.ui
 )

 qt5_wrap_ui(FORMS_HEADERS ${FORMS})


add_library(COS SHARED
    ${FORMS_HEADERS}
    cos_list_object_response.h cos_list_object_response.cpp
    cos_global.h export.h export.cpp
    cos_form_dir_setting.h cos_form_dir_setting.cpp
    cos_form_general.h cos_form_general.cpp
    cos_panel.h cos_panel.cpp
    cos_parse.h cos_parse.cpp
    cos_response.h cos_response.cpp
    cos_setting_key.h cos_setting.h cos_setting.cpp
    cos_worker.h cos_worker.cpp
    cos.h cos.cpp
)

target_link_libraries(COS PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql App ${CRYPTO_LIB} ${SSL_LIB}  ${CURL_LIB} )

target_compile_definitions(COS PRIVATE COS_LIBRARY)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll  ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll
    #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/language.zh_CN.qm  ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/language.zh_CN.qm
    COMMENT "Copying file to binary directory"
)

