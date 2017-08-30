cmake_minimum_required(VERSION 3.3)
project(test_build)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ldl -lpthread -lfcgi")
add_library(InnerRequest InnerRequest.cpp)
add_library(json json.hpp json.cpp)

set(SOURCE_FILES
    #[[plugins/diary.cpp]]
    core.cpp
    core.hpp
    main.cpp)

add_executable(test_build ${SOURCE_FILES} InnerRequest)

add_subdirectory(plugins)
