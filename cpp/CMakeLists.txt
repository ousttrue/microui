# project
cmake_minimum_required(VERSION 3.20.0)
project(hello VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
add_definitions(-DNOMINMAX)

# target
subdirs(uirf _external/glfw microui _external/glad gl_renderer demo)
