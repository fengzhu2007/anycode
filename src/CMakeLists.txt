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
  list(APPEND ANYCODE_CFILES copyright.rc)
elseif(APPLE)
    set(APP_ICON_PATH "${CMAKE_SOURCE_DIR}/src/logo.icns")
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AnyCode
        MANUAL_FINALIZATION
        ${ANYCODE_SOURCES}
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



