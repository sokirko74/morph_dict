declare_cmake_min_version()

project(test_morph_gen)

add_executable (${PROJECT_NAME} test_morph_gen.cpp )

target_link_libraries(${PROJECT_NAME} 
    lemmatizer_base_lib
)


add_test(
        NAME MorpGenTest 
        COMMAND test_morph_gen
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

