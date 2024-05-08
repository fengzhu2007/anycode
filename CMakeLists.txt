cmake_minimum_required(VERSION 3.5)

project(anycode VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)


set(ANYCODE_SOURCES
        src/main.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(anycode
        MANUAL_FINALIZATION
        ${ANYCODE_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET anycode APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(anycode SHARED
            ${ANYCODE_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(anycode
            ${ANYCODE_SOURCES}
        )
    endif()
endif()





add_subdirectory(App)
add_subdirectory(addon)





target_link_libraries(anycode PRIVATE Qt${QT_VERSION_MAJOR}::Widgets App)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.anycode)
endif()
set_target_properties(anycode PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS anycode
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(anycode)
endif()


#include_directories(SYSTEM "libraries/SDL/include")
#add_subdirectory("libraries/SDL")

#include_directories(SYSTEM "libraries/fmt/include")
#add_subdirectory("libraries/fmt")

#add_subdirectory("libraries/glew/build/cmake")

#include_directories(SYSTEM "libraries/glad/include")


#add_subdirectory(subdir) 添加子cmake文件






#anycode
#set(ANYCODE_EXE_SOURCES
#        src/main.cpp
##)
#add_executable(anycode ${ANYCODE_EXE_SOURCES})
#target_link_libraries(anycode PRIVATE App)

#if(WIN32)
#    set_target_properties(anycode PROPERTIES WIN32_EXECUTABLE TRUE)
#endif()



