cmake_minimum_required(VERSION 3.16)

project(python LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Sql)


include_directories(SYSTEM "../../App")
include_directories(SYSTEM "../../texteditor")

add_library(python SHARED
  python_global.h
  python.h
  python.cpp
  loader.h
  loader.cpp
  pythonscanner.h
  pythonscanner.cpp
  pythonhighlighter.h
  pythonhighlighter.cpp
  pythonindenter.h
  pythonindenter.cpp
  pythonautocompleter.h
  pythonautocompleter.cpp
  pythoncodeformatter.h
  pythoncodeformatter.cpp
)

target_link_libraries(python PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql App texteditor)

target_compile_definitions(python PRIVATE PYTHON_LIBRARY)

if(WIN32)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/${PROJECT_NAME}.dll
    #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/addon/${PROJECT_NAME}/language.zh_CN.qm ${CMAKE_BINARY_DIR}/src/addon/${PROJECT_NAME}/language.zh_CN.qm
    COMMENT "Copying file to binary directory"
)
elseif()

endif()
