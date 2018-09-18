add_library(mathlang mathtype.cpp mathtype.hpp terms.hpp terms.cpp structure.cpp structure.hpp rationale.cpp rationale.hpp
        old_parser.cpp old_parser.hpp view.cpp view.hpp lexer.cpp lexer.hpp parser.cpp parser.hpp)
set_target_properties(mathlang PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_subdirectory(parser_tests)
