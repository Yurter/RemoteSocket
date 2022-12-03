cmake_minimum_required(VERSION 3.14)

project(RemoteSocket LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core)
find_package(Qt5 REQUIRED COMPONENTS Network)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(SetOutputPaths)
include(Pack)
SetOutputPaths()

add_executable(RemoteSocket
    main.cpp
)

target_link_libraries(RemoteSocket
    Qt5::Core
    Qt5::Network
)

if(WIN32)
    set(CMAKE_INSTALL_BINDIR .)
    include(${CMAKE_SOURCE_DIR}/../cmake/Windeployqt.cmake)
    set(WINDEPLOYQT_ARGS --no-system-d3d-compiler)
    windeployqt(RemoteSocket ${CMAKE_INSTALL_BINDIR})
endif(WIN32)

install(TARGETS RemoteSocket DESTINATION . COMPONENT ${COMPONENT_NAME})


##################################### Pack #####################################

if(WIN32)
    set(CPACK_GENERATOR "7Z")
else()
    set(CPACK_GENERATOR "DEB")
endif()

Pack()
