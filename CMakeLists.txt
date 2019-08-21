cmake_minimum_required(VERSION 3.15)
project(open-rpg
        VERSION 0.1
        DESCRIPTION "Hello"
        LANGUAGES C)

set(CMAKE_C_STANDARD 11)

set(RPG_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/RPG
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include)

include_directories(${RPG_INCLUDE_DIRS})

#find_package(Ruby 2 REQUIRED)
#include_directories(${RUBY_INCLUDE_DIRS})

add_subdirectory(src)
add_subdirectory(lib)