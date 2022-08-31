# project
cmake_minimum_required(VERSION 3.20.0)
project(hello VERSION 0.1.0)
SET(CMAKE_C_STANDARD 11)

# target
subdirs(_external/glfw src demo)
