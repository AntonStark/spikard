add_executable(logic-test test.cpp)
target_link_libraries(logic-test LINK_PUBLIC mathlang)
target_link_libraries(logic-test LINK_PUBLIC json)

add_library(math MODULE mathlang.cpp)
target_link_libraries(math LINK_PUBLIC mathlang)
set_target_properties(math PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/plugins
        PREFIX ""
        OUTPUT_NAME math)
