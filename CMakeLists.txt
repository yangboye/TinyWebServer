# cmake_minimum_required(VERSION <specify CMake version here>)
project(TinyWebServer)

set(CMAKE_CXX_STANDARD 11)

add_executable(TinyWebServer main.cpp lock/locker.h)