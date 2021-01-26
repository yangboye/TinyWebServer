# cmake_minimum_required(VERSION <specify CMake version here>)
project(TinyWebServer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS -pthread)

add_executable(TinyWebServer main.cpp src/lock/locker.h test/test_semaphore.h src/log/block_queue.h src/log/log.cpp src/log/log.h)