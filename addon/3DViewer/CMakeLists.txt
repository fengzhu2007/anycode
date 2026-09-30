cmake_minimum_required(VERSION 3.14)

project(3DViewer LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql 3DCore 3DRender 3DInput 3DExtras)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql 3DCore 3DRender 3DInput 3DExtras)

include_directories(SYSTEM "../../App")
include_directories(SYSTEM "../../wRect/src")

add_library(3DViewer SHARED
    3dviewer_global.h
    export.h
    export.cpp
    viewer3dwidget.h
    viewer3dwidget.cpp
    viewer3dwindow.h
    viewer3dwindow.cpp
    viewer3dwindow.ui
)

target_link_libraries(3DViewer PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::3DCore
    Qt${QT_VERSION_MAJOR}::3DRender
    Qt${QT_VERSION_MAJOR}::3DInput
    Qt${QT_VERSION_MAJOR}::3DExtras
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
