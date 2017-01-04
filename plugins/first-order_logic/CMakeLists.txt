add_library(logic logic.hpp logic.cpp)
add_library(signature signature.hpp signature.cpp interpr.cpp)

add_executable(logic-test test.cpp)
target_link_libraries(logic-test LINK_PUBLIC logic signature)