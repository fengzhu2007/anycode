cmake_minimum_required(VERSION 3.10)
project(anycodeSolution VERSION 0.9 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)



set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DUNICODE")



#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# 添加 DependencyProject 子目录
add_subdirectory(3party)

add_subdirectory(texteditor)
#add_subdirectory(3party/qscintilla/src)
add_subdirectory(dockingpane)
add_subdirectory(wRect)
#add_subdirectory(react-qt)

add_subdirectory(App)

add_subdirectory(libsvnpp)


add_subdirectory(addon)



# 添加 MainProject 子目录
add_subdirectory(src)


#add_custom_command(TARGET your_target_name POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy
#        ${CMAKE_SOURCE_DIR}/source_file.txt
#        ${CMAKE_BINARY_DIR}/destination_file.txt
#    COMMENT "Copying file to binary directory"
#)
