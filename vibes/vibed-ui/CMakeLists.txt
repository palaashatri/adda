cmake_minimum_required(VERSION 3.16)
project(UIToolkit LANGUAGES C CXX)

if(APPLE)
    enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /EHs-c- /GR-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -fno-exceptions -fno-rtti)
endif()

add_subdirectory(src)
add_subdirectory(examples)