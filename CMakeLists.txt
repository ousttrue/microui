# project
cmake_minimum_required(VERSION 3.20.0)
project(hello VERSION 0.1.0)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
add_definitions(-DNOMINMAX)

# target
subdirs(uirf _external/glfw src demo)
