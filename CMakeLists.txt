project(classification)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_CXX_FLAGS "-Wall")

find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} external/include)

add_subdirectory( src )
add_subdirectory( external )
