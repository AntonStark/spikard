add_library(mathlang basics/mathtype.cpp basics/mathtype.hpp basics/terms.hpp basics/terms.cpp structure.cpp structure.hpp rationale.cpp rationale.hpp
        parser/old_parser.cpp parser/old_parser.hpp view.cpp view.hpp parser/lexer.cpp parser/lexer.hpp parser/parser.cpp parser/parser.hpp)
set_target_properties(mathlang PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_subdirectory(parser_tests)
