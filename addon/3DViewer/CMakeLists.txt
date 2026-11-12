cmake_minimum_required(VERSION 3.14)

project(3DViewer LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# 3D 预览基于 Qt Quick 3D 的 RuntimeLoader（Qt 6.2 起提供），
# 低版本 Qt 直接跳过该 addon，避免影响整体构建。
if(QT_VERSION VERSION_LESS 6.2)
    message(WARNING "3DViewer addon requires Qt 6.2 or newer (Qt Quick 3D RuntimeLoader), skipped for Qt ${QT_VERSION}.")
    return()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickWidgets Quick3D)

include_directories(SYSTEM "../../App")
include_directories(SYSTEM "../../wRect/src")

add_library(3DViewer SHARED
    3dviewer_global.h
    export.h
    export.cpp
    viewer3dconfig.h
    viewer3dconfig.cpp
    viewer3dthumbcache.h
    viewer3dthumbcache.cpp
    viewer3dconverter.h
    viewer3dconverter.cpp
    viewer3dthumbnailrenderer.h
    viewer3dthumbnailrenderer.cpp
    viewer3dwidget.h
    viewer3dwidget.cpp
    viewer3dwindow.h
    viewer3dwindow.cpp
    3dviewer.qrc
)

target_link_libraries(3DViewer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickWidgets
    Qt6::Quick3D
    App
    wRect
)

target_compile_definitions(3DViewer PRIVATE VIEWER3D_LIBRARY)

if(WIN32)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy  ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll
    COMMENT "Copying file to binary directory"
)

elseif()

endif()
