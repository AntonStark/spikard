project(spikard)
cmake_minimum_required(VERSION 3.3)

add_library(json STATIC json.hpp json.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ldl -lpthread -lfcgi")
set(SOURCE_FILES
    core.cpp
    core.hpp
    main.cpp)

add_executable(spikard-core ${SOURCE_FILES})
target_link_libraries(spikard-core json)

add_subdirectory(lib)
add_subdirectory(plugins)
