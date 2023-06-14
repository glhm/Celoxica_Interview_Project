cmake_minimum_required(VERSION 3.10)

set(PROJECT_N Celo)

project(${PROJECT_N} VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE SRCS ${CMAKE_SOURCE_DIR}/src/*.cpp)

include_directories(${CMAKE_SOURCE_DIR}/inc)

add_executable(${PROJECT_N} ${SRCS})
