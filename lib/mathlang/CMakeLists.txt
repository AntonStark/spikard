add_library(mathlang basics/mathtype.cpp basics/mathtype.hpp basics/terms.hpp basics/terms.cpp basics/mapterm.cpp basics/mapterm.hpp basics/contructedterm.cpp basics/contructedterm.hpp
        proof/structure.cpp proof/structure.hpp proof/names_index.cpp proof/names_index.hpp proof/names_strategies.cpp proof/names_strategies.hpp
        proof/definition.cpp proof/definition.hpp proof/statement.cpp proof/statement.hpp
        proof/named_node.cpp proof/named_node.hpp view.cpp view.hpp
        parser/lexer.cpp parser/lexer.hpp parser/parser.cpp parser/parser.hpp)
set_target_properties(mathlang PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_subdirectory(parser_tests)
