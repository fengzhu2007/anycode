cmake_minimum_required(VERSION 3.14)



set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(libsvnpp SHARED
        src/svnpp_client.cpp
		src/client/diff_summary.cpp
		src/client/info.cpp
		src/client/log_entry.cpp
)

target_link_libraries(libsvnpp PRIVATE Qt${QT_VERSION_MAJOR}::Core)

target_compile_definitions(libsvnpp PRIVATE APP_LIBRARY)
