# project
cmake_minimum_required(VERSION 3.20.0)
project(hello VERSION 0.1.0)

# target
subdirs(_external/SDL2-2.24.0 src demo)
