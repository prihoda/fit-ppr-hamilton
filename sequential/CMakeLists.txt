cmake_minimum_required(VERSION 3.3)
project(PPR_Sequential)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp Graph.cpp Graph.h MaxHamilton.cpp MaxHamilton.h)
add_executable(PPR_Sequential ${SOURCE_FILES})