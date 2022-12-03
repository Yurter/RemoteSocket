cmake_minimum_required(VERSION 3.14)

project(RemoteSocketServer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core)
find_package(Qt5 REQUIRED COMPONENTS Network)

add_executable(RemoteSocketServer
  main.cpp
)
target_link_libraries(RemoteSocketServer
    Qt5::Core
    Qt5::Network
)
