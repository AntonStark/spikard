cmake_minimum_required(VERSION 3.3)
project(test_build)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ldl -lpthread")
add_library(XHR.h XHR)

set(SOURCE_FILES
    plugins/diary.cpp
    core.cpp
    core.hpp
    main.cpp)

add_executable(test_build ${SOURCE_FILES} XHR)