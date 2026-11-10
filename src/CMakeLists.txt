cmake_minimum_required(VERSION 3.10)

project(AnyCode VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MACOSX_BUNDLE TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Xml Sql Network)
#find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Xml Sql Network)

#find_package(DbgHelp)

include_directories(SYSTEM "../App")
include_directories(SYSTEM "../wRect/src")


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DQ_DEBUG)
endif()


set(ANYCODE_SOURCES
        main.cpp
)



if(WIN32)
    # Windows 下的资源脚本（EXE 图标 + 版本信息）
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND ANYCODE_CFILES copyrightd.rc)
    else()
        list(APPEND ANYCODE_CFILES copyright.rc)
    endif()
elseif(APPLE)
    set(APP_ICON_PATH "${CMAKE_SOURCE_DIR}/src/logo.icns")
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AnyCode
        MANUAL_FINALIZATION
        ${ANYCODE_SOURCES}
        ${ANYCODE_CFILES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET anycode APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(AnyCode SHARED
            ${ANYCODE_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(AnyCode
            ${ANYCODE_SOURCES}
            ${ANYCODE_CFILES}
        )
    endif()
endif()



if(CMAKE_HOST_WIN32)


target_link_libraries(AnyCode PRIVATE Qt${QT_VERSION_MAJOR}::Widgets DbgHelp App wRect)

else()
target_link_libraries(AnyCode PRIVATE Qt${QT_VERSION_MAJOR}::Widgets App wRect)

endif()



#message("dir is ${CMAKE_SOURCE_DIR}")
#set_target_properties(anycode PROPERTIES WIN32_ICON "${CMAKE_SOURCE_DIR}/others/logo.ico")

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.AnyCode)
endif()
set_target_properties(AnyCode PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS AnyCode
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(AnyCode)
endif()


if(APPLE)
    file(COPY ${APP_ICON_PATH} DESTINATION ${CMAKE_BINARY_DIR}/AnyCode.app/Contents/Resources/)
endif()


# ---------------------------------------------------------------------------
# Windows 运行时 Qt 插件部署
#
# Qt6 官方发行版是可重定位(relocatable)构建，QLibraryInfo 会从 Qt6Core.dll 的
# 所在位置反推 prefix。本项目把 Qt 的 DLL 直接放在可执行文件同级目录（而不是
# <prefix>/bin），导致 prefix 推导结果为空，插件搜索路径也随之变成空字符串，
# 运行时报错：Could not find the Qt platform plugin "windows" in ""
#
# 这里在可执行文件旁生成 qt.conf，并把 Qt 插件复制到 <exe>/plugins 目录，
# 使插件路径不再依赖 prefix 推导。
# ---------------------------------------------------------------------------
if(WIN32)
    set(ANYCODE_QT_PLUGIN_DIRS
        platforms
        platforminputcontexts
        imageformats
        iconengines
        styles
        sqldrivers
        tls
        networkinformation
        generic
    )

    # 定位 Qt 插件源目录：<qt>/bin/qmake.exe -> <qt>/plugins
    set(ANYCODE_QT_PLUGINS_DIR "")
    get_target_property(ANYCODE_QMAKE_LOCATION Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
    if(ANYCODE_QMAKE_LOCATION)
        get_filename_component(_anycode_qt_bin_dir "${ANYCODE_QMAKE_LOCATION}" DIRECTORY)
        get_filename_component(ANYCODE_QT_PLUGINS_DIR "${_anycode_qt_bin_dir}/../plugins" ABSOLUTE)
    endif()

    # qt.conf：插件相对可执行文件目录解析，与 prefix 推导无关
    set(ANYCODE_QT_CONF "${CMAKE_CURRENT_BINARY_DIR}/qt.conf")
    file(WRITE "${ANYCODE_QT_CONF}" "[Paths]\nPrefix = .\nPlugins = plugins\n")

    add_custom_command(TARGET AnyCode POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ANYCODE_QT_CONF}"
            "$<TARGET_FILE_DIR:AnyCode>/qt.conf"
        COMMENT "Deploying qt.conf"
    )

    if(ANYCODE_QT_PLUGINS_DIR AND IS_DIRECTORY "${ANYCODE_QT_PLUGINS_DIR}")
        message(STATUS "Deploying Qt plugins from ${ANYCODE_QT_PLUGINS_DIR}")
        foreach(_anycode_plugin_dir ${ANYCODE_QT_PLUGIN_DIRS})
            file(GLOB _anycode_plugin_files "${ANYCODE_QT_PLUGINS_DIR}/${_anycode_plugin_dir}/*.dll")
            if(_anycode_plugin_files)
                add_custom_command(TARGET AnyCode POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory
                        "$<TARGET_FILE_DIR:AnyCode>/plugins/${_anycode_plugin_dir}"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${_anycode_plugin_files}
                        "$<TARGET_FILE_DIR:AnyCode>/plugins/${_anycode_plugin_dir}"
                    COMMENT "Deploying Qt plugin: ${_anycode_plugin_dir}"
                )
            endif()
        endforeach()
    else()
        message(WARNING "Qt plugins directory not found, skip plugin deployment: '${ANYCODE_QT_PLUGINS_DIR}'")
    endif()
endif()



