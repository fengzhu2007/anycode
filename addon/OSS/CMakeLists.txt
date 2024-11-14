project(OSS)

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
include_directories(SYSTEM "../../3party/openssl/crypto")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DQ_DEBUG)
endif()
set(FORMS
    oss_form_dir_setting.ui
        oss_form_general.ui

)

qt5_wrap_ui(FORMS_HEADERS ${FORMS})


add_library(OSS SHARED
    ${FORMS_HEADERS}
    oss_global.h export.h export.cpp
    oss_form_dir_setting.h oss_form_dir_setting.cpp
    oss_form_general.h oss_form_general.cpp
    oss_response.h oss_response.cpp
    oss_setting_key.h oss_setting.h oss_setting.cpp
    oss.h oss.cpp
)

target_link_libraries(OSS PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql App ${CRYPTO_LIB} ${SSL_LIB}  ${CURL_LIB})

target_compile_definitions(OSS PRIVATE OSS_LIBRARY)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll
    #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/language.zh_CN.qm ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/language.zh_CN.qm
    COMMENT "Copying file to binary directory"
)
