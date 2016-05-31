cmake_minimum_required(VERSION 3.4)

project(wii-server)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/../bin)
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set (EXT_LIB "${PROJECT_SOURCE_DIR}/../lib")
include_directories(${PROJECT_SOURCE_DIR} ${EXT_LIB})

add_library(wiiuse SHARED IMPORTED)
set_property(TARGET wiiuse PROPERTY IMPORTED_LOCATION "/usr/local/lib/libwiiuse.dylib")

add_library(protobuf_lib STATIC IMPORTED)
set_property(TARGET protobuf_lib PROPERTY IMPORTED_LOCATION /usr/local/lib/libprotobuf.a)

add_subdirectory(easywsclient)
add_subdirectory(protobuf)

add_executable(wii-server wii-server.cpp)
target_link_libraries(wii-server wiiuse easywsclient protobuf_lib VRCom)
target_compile_features(wii-server PUBLIC cxx_range_for)

