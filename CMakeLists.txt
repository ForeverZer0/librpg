cmake_minimum_required(VERSION 3.15)

add_definitions("-Wall -DRPG_DEBUG")

project(open-rpg
        VERSION 0.1
        DESCRIPTION "Hello"
        LANGUAGES C)

set(CMAKE_C_STANDARD 11)

set(RPG_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include/RPG)
set(RPG_LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")

include_directories(${RPG_INCLUDE_DIRS})


# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${RPG_LIBS_DIR}/glfw)


add_subdirectory(src)
add_subdirectory(lib)