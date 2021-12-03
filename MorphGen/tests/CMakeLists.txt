declare_cmake_min_version()

project(MorphGenTest)

add_executable (${PROJECT_NAME}
   TestMorphGen.cpp
)

target_link_libraries(${PROJECT_NAME} 
    LemmatizerBaseLib
)

# test morphgen for a simple dictionary
macro (TestMorphGen lang path)
    get_filename_component(base_name ${path} NAME)
    get_filename_component(folder ${path} DIRECTORY)
    message (${folder})
    add_test(
            NAME MorphGen_${lang}_${base_name}_0
            COMMAND MorphGen --input ${path} --output-folder ${CMAKE_CURRENT_BINARY_DIR} --postfix-len 5 --min-freq 3
    )

    add_test(
            NAME MorphGen_${lang}_${base_name}_1
            COMMAND MorphGenTest ${CMAKE_CURRENT_BINARY_DIR} "дума" "${folder}/test_word.morph.canon" 
    )
endmacro(TestMorphGen)

TestMorphGen( Russian   ${PROJECT_SOURCE_DIR}/Russian/project.mwz)

