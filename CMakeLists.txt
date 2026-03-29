cmake_minimum_required(VERSION 3.15)
project(muhendisC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Kaynak dosyalar
set(SOURCES
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/compiler.cpp
    src/vm.cpp
)

# Executable
add_executable(muhendisC ${SOURCES})

# Include path
target_include_directories(muhendisC PRIVATE src)

# MSVC icin UTF-8 destegi
if(MSVC)
    target_compile_options(muhendisC PRIVATE /utf-8)
endif()

# Release build icin optimizasyon
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        target_compile_options(muhendisC PRIVATE /O2)
    else()
        target_compile_options(muhendisC PRIVATE -O2)
    endif()
endif()
