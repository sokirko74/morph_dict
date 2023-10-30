declare_cmake_min_version()

project(morph_gen)

add_executable (${PROJECT_NAME}
        morph_gen.cpp
)

target_link_libraries(${PROJECT_NAME} 
    lemmatizer_base_lib
)

add_subdirectory (tests/Lemmatize)
add_subdirectory (tests/Misspell)
