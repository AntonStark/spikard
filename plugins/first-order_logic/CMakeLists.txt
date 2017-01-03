add_library(logic.hpp logic.cpp logic)
add_library(signature.hpp signature.cpp signature)
add_executable(logic-test test.cpp logic signature)