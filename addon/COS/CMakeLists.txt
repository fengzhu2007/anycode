project(COS)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql LinguistTools )
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql LinguistTools )


# curl 头文件直接取自 3party 源码目录，库由顶层 3party 的 in-tree 目标提供（ADDON_CURL_LIB）
# 签名用的 HMAC 已改用 Qt 内置的 QMessageAuthenticationCode，不再依赖 openssl
include_directories(SYSTEM "../../App")
include_directories(SYSTEM "../../3party/curl/include")


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DQ_DEBUG)
endif()

 set(FORMS
        cos_form_dir_setting.ui
        cos_form_general.ui
 )

# Qt5/Qt6 兼容：Qt6 未提供 qt5_wrap_ui
if(QT_VERSION_MAJOR EQUAL 6)
    qt6_wrap_ui(FORMS_HEADERS ${FORMS})
else()
    qt5_wrap_ui(FORMS_HEADERS ${FORMS})
endif()


add_library(COS SHARED
    ${FORMS_HEADERS}
    cos_list_object_response.h cos_list_object_response.cpp
    cos_global.h export.h export.cpp
    cos_form_dir_setting.h cos_form_dir_setting.cpp
    cos_form_general.h cos_form_general.cpp
    cos_parse.h cos_parse.cpp
    cos_response.h cos_response.cpp
    cos_setting_key.h cos_setting.h cos_setting.cpp
    cos.h cos.cpp
)

target_link_libraries(COS PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql App ${ADDON_CURL_LIB})

target_compile_definitions(COS PRIVATE COS_LIBRARY)


if(WIN32)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll  ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll
    #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/language.zh_CN.qm  ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/language.zh_CN.qm
    COMMENT "Copying file to binary directory"
)
elseif()

endif()

