add_library(mathlang logic.hpp logic.cpp structure.cpp structure.hpp rationale.cpp rationale.hpp parser.cpp parser.hpp view.cpp view.hpp)
set_target_properties(mathlang PROPERTIES POSITION_INDEPENDENT_CODE ON)
