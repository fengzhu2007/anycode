
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network)

if(WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DUNICODE /D_UNICODE")
endif()

set(SOURCES
    iptyprocess.h
    ptyqt.cpp ptyqt.h
)

if (WIN32)
    list(APPEND SOURCES
        winptyprocess.cpp winptyprocess.h
        conptyprocess.cpp conptyprocess.h
    )
else()
    list(APPEND SOURCES unixptyprocess.cpp unixptyprocess.h)
endif()

add_library(ptyqt STATIC ${SOURCES})

if(WIN32)
target_link_libraries(ptyqt PUBLIC Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network winpty)
else()
    target_link_libraries(ptyqt PUBLIC Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network)

endif()

if (WIN32)
  #target_link_libraries(ptyqt PRIVATE  )
  #target_compile_definitions(ptyqt PRIVATE PTYQT_DEBUG)
endif()

set_target_properties(ptyqt
  PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}
    QT_COMPILE_OPTIONS_DISABLE_WARNINGS ON
    POSITION_INDEPENDENT_CODE ON
)
