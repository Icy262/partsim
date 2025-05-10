cmake_minimum_required(VERSION 4.0.2)
project(partsim VERSION 0.0.0 LANGUAGES C)
add_executable(partsim main.c)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw-3.4)
target_link_libraries(partsim glfw)
find_package(OpenGL REQUIRED)
target_link_libraries(partsim OpenGL::GL)