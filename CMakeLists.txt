cmake_minimum_required(VERSION 3.10)
include(CMakePrintHelpers)

project(cxx-d3d11 VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(DXTK/Inc)

add_executable(${PROJECT_NAME} WIN32
    src/main.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/DXTK/Bin/Windows10_2022/x64/Debug/DirectXTK.lib)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
endif()
