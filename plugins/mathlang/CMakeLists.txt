add_executable(logic-test test.cpp)
target_link_libraries(logic-test LINK_PUBLIC mathlang)
target_link_libraries(logic-test LINK_PUBLIC json)

add_library(mathlang-plugin MODULE mathlang.cpp)
target_link_libraries(mathlang-plugin LINK_PUBLIC mathlang)
set_target_properties(mathlang-plugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/plugins
        PREFIX ""
        OUTPUT_NAME mathlang)
