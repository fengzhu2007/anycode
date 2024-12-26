cmake_minimum_required(VERSION 3.14)

project(S3)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql LinguistTools)

 if(WIN32)
find_library(CRYPTO_LIB libcrypto PATHS "../3party/bin/x64/lib")
find_library(SSL_LIB libssl PATHS "../3party/bin/x64/lib")
find_library(CURL_LIB libcurl_imp PATHS "../3party/bin/x64/lib")
else()
find_library(CRYPTO_LIB crypto PATHS "../3party/bin/x64/lib")
find_library(SSL_LIB ssl PATHS "../3party/bin/x64/lib")
find_library(CURL_LIB curl PATHS "../3party/bin/x64/lib")
endif()

include_directories(SYSTEM "../../App")
include_directories(SYSTEM "../../3party/bin/x64/include")
include_directories(SYSTEM "../../3party/openssl/crypto")


set(FORMS
    s3_form_dir_setting.ui
    s3_form_general.ui

)

qt5_wrap_ui(FORMS_HEADERS ${FORMS})



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

target_link_libraries(S3 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql App ${CRYPTO_LIB} ${SSL_LIB}  ${CURL_LIB})

target_compile_definitions(S3 PRIVATE S3_LIBRARY)


if(WIN32)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy  ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll
    COMMENT "Copying file to binary directory"
)

elseif()

endif()
