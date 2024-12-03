project(FTP)

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

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DQ_DEBUG)
endif()

set(FORMS
    ftp_form_dir_setting.ui
    ftp_form_general.ui
)

qt5_wrap_ui(FORMS_HEADERS ${FORMS})



add_library(FTP SHARED
    ${FORMS_HEADERS}

    ftp_global.h
    export.h export.cpp
    ftp_form_dir_setting.h ftp_form_dir_setting.cpp
    ftp_form_general.h ftp_form_general.cpp
    ftp_parse.h ftp_parse.cpp
    ftp_response.h ftp_response.cpp
    ftp_setting.h ftp_setting_key.h ftp_setting.cpp
    ftp.h ftp.cpp
)

target_link_libraries(FTP PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql App ${CRYPTO_LIB} ${SSL_LIB}  ${CURL_LIB})

target_compile_definitions(FTP PRIVATE FTP_LIBRARY)


if(WIN32)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy  ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll
    #COMMAND ${CMAKE_COMMAND} -E copy  ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/language.zh_CN.qm  ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/language.zh_CN.qm
    COMMENT "Copying file to binary directory"
)

elseif()

endif()
