add_library(logic logic.hpp logic.cpp structure.cpp structure.hpp rationale.cpp rationale.hpp parser.cpp parser.hpp view.cpp view.hpp)

add_executable(logic-test test.cpp)
target_link_libraries(logic-test LINK_PUBLIC logic)
target_link_libraries(logic-test LINK_PUBLIC json)

add_executable(mathlang_plugin mathlang.cpp)
target_link_libraries(mathlang_plugin LINK_PUBLIC logic)
