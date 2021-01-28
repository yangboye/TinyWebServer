# cmake_minimum_required(VERSION <specify CMake version here>)
cmake_minimum_required(VERSION 3.16)
project(TinyWebServer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS -pthread)
include_directories(/usr/include/)

add_executable(TinyWebServer main.cpp src/lock/locker.h test/test_semaphore.h src/log/block_queue.h src/log/log.cpp src/log/log.h src/cgi_mysql/sql_connection_pool.cpp src/cgi_mysql/sql_connection_pool.h)