add_library(logic logic.hpp logic.cpp formulas.hpp formulas.cpp signature.cpp signature.hpp parser.cpp parser.hpp)

add_executable(logic-test test.cpp)
target_link_libraries(logic-test LINK_PUBLIC logic)