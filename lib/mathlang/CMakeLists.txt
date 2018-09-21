add_library(mathlang basics/mathtype.cpp basics/mathtype.hpp basics/terms.hpp basics/terms.cpp
        structure.cpp structure.hpp names.cpp names.hpp names_strategies.cpp names_strategies.hpp
        definition.cpp definition.hpp statement.cpp statement.hpp
        named_node.cpp named_node.hpp primary_node.cpp primary_node.hpp branch_node.cpp branch_node.hpp
        view.cpp view.hpp parser/lexer.cpp parser/lexer.hpp parser/parser.cpp parser/parser.hpp)
set_target_properties(mathlang PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_subdirectory(parser_tests)
