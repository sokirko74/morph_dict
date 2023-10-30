declare_cmake_min_version()

project(morph_genTest)

add_executable (${PROJECT_NAME}
   TestMorphGen.cpp
)

target_link_libraries(${PROJECT_NAME} 
    lemmatizer_base_lib
)

# test morphgen for a simple dictionary
macro (Testmorph_gen lang path)
    get_filename_component(base_name ${path} NAME)
    get_filename_component(folder ${path} DIRECTORY)
    message (${folder})
    add_test(
            NAME morph_gen_${lang}_${base_name}_0
            COMMAND morph_gen --input ${path} --output-folder ${CMAKE_CURRENT_BINARY_DIR} --postfix-len 5 --min-freq 3
    )

    add_test(
            NAME morph_gen_${lang}_${base_name}_1
            COMMAND morph_genTest ${CMAKE_CURRENT_BINARY_DIR} "дума" "${folder}/test_word.morph.canon"
    )
endmacro()

Testmorph_gen( Russian   ${PROJECT_SOURCE_DIR}/Russian/project.mwz)

