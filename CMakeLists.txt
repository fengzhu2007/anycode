cmake_minimum_required(VERSION 3.5)
project(anycodeSolution)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)



# 添加 DependencyProject 子目录
add_subdirectory(App)

add_subdirectory(libsvnpp)


add_subdirectory(addon)

add_subdirectory(dockingpane)
add_subdirectory(wRect)

# 添加 MainProject 子目录
add_subdirectory(src)


#add_custom_command(TARGET your_target_name POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy
#        ${CMAKE_SOURCE_DIR}/source_file.txt
#        ${CMAKE_BINARY_DIR}/destination_file.txt
#    COMMENT "Copying file to binary directory"
#)
