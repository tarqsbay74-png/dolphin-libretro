cmake_minimum_required(VERSION 3.10)
project(DolphinLibretro)

set(CMAKE_CXX_STANDARD 17)

option(ENABLE_GENERIC "Enable generic build without JIT" ON)

if(ANDROID)
    if(ENABLE_GENERIC)
        add_definitions(-DUSE_GENERIC)
        add_definitions(-DNO_CURL)
    else()
        message(FATAL_ERROR "You're building on an unsupported platform. Enable generic build if you really want a JIT-less binary.")
    endif()
endif()

add_library(dolphin_libretro_android SHARED
    Source/Core/Core/Core.cpp
    # Add all required source files here
)

target_include_directories(dolphin_libretro_android PRIVATE
    ${CMAKE_SOURCE_DIR}/Source/Core
    ${CMAKE_SOURCE_DIR}/Source
)
