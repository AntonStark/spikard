add_library(mathlang mathtype.cpp mathtype.hpp terms.hpp terms.cpp structure.cpp structure.hpp rationale.cpp rationale.hpp parser.cpp parser.hpp view.cpp view.hpp lexer2.cpp lexer2.hpp)
set_target_properties(mathlang PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_subdirectory(parser_tests)
