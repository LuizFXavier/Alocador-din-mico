cmake_minimum_required(VERSION 3.28)
project(alocador)

set(CMAKE_CXX_STANDARD 23)

add_executable(alocador
src/GC.cpp
src/Main.cpp)

target_include_directories(alocador PRIVATE include)