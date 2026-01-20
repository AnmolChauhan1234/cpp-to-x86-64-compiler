cmake_minimum_required(VERSION 3.20)
project(mycompiler)
set(CMAKE_CXX_STANDARD 20)

# If CMAKE_BUILD_TYPE is Debug, add debug flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
endif()

add_executable(mycompiler src/main.cpp)
