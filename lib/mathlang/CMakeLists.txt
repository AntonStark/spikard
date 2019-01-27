add_library(mathlang basics/mathtype.cpp basics/mathtype.hpp consepts/terms.hpp consepts/terms.cpp basics/mapterm.cpp basics/mapterm.hpp basics/contructed_term.cpp basics/contructed_term.hpp
        proof/structure.cpp proof/structure.hpp proof/names_index.cpp proof/names_index.hpp proof/names_strategies.cpp proof/names_strategies.hpp
        proof/definition.cpp proof/definition.hpp proof/statement.cpp proof/statement.hpp
        proof/named_node.cpp proof/named_node.hpp view.cpp view.hpp
        parser/lexer.cpp parser/lexer.hpp parser/parser.cpp parser/parser.hpp parser/lexeme.cpp parser/lexeme.hpp
        basics/named_term.cpp basics/named_term.hpp consepts/abstract_name.cpp consepts/abstract_name.hpp basics/texname.cpp basics/texname.hpp  basics/string_name.cpp basics/string_name.hpp
        consepts/abstract_connective.cpp consepts/abstract_connective.hpp basics/function.cpp basics/function.hpp basics/complex.cpp basics/complex.hpp basics/connectives.cpp basics/connectives.hpp proof/theorem_node.cpp proof/theorem_node.hpp)
set_target_properties(mathlang PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_subdirectory(parser_tests)
