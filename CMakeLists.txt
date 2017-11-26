cmake_minimum_required(VERSION 3.3)
project(test_build)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ldl -lpthread -lfcgi")
add_library(json json.hpp json.cpp)

set(SOURCE_FILES
    core.cpp
    core.hpp
    main.cpp)

add_executable(test_build ${SOURCE_FILES} json)

add_subdirectory(lib)
add_subdirectory(plugins)
