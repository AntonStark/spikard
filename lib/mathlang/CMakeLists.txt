add_library(mathlang
        consepts/terms.hpp consepts/terms.cpp consepts/abstract_name.cpp consepts/abstract_name.hpp consepts/abstract_connective.cpp consepts/abstract_connective.hpp

        basics/primary.cpp basics/primary.hpp basics/complex.cpp basics/complex.hpp
        basics/texname.cpp basics/texname.hpp  basics/string_name.cpp basics/string_name.hpp
        basics/connectives.cpp basics/connectives.hpp basics/function.cpp basics/function.hpp

        parser/lexeme.cpp parser/lexeme.hpp parser/lexer.cpp parser/lexer.hpp parser/parser.cpp parser/parser.hpp

        proof/names_index.cpp proof/names_index.hpp proof/names_strategies.cpp proof/names_strategies.hpp
        proof/structure.cpp proof/structure.hpp proof/named_node.cpp proof/named_node.hpp view.cpp view.hpp
        proof/definition.cpp proof/definition.hpp proof/statement.cpp proof/statement.hpp
        proof/theorem_node.cpp proof/theorem_node.hpp)
set_target_properties(mathlang PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_subdirectory(parser_tests)
