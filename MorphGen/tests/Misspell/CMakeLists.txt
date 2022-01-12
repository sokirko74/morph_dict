declare_cmake_min_version()

project(MisspellTest)

add_executable (${PROJECT_NAME}
   TestMisspell.cpp
)

target_link_libraries(${PROJECT_NAME} 
    LemmatizerBaseLib
)

# test morphgen for a simple dictionary
macro (TestMisspellMacro lang path)
    get_filename_component(base_name ${path} NAME)
    get_filename_component(folder ${path} DIRECTORY)
    message (${folder})
    add_test(
            NAME Misspell_${lang}_gen
            COMMAND MorphGen --input ${path} --output-folder ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_test(
            NAME Misspell_${lang}_test
            COMMAND MisspellTest ${CMAKE_CURRENT_BINARY_DIR} 
    )
endmacro()

TestMisspellMacro(Russian   ${PROJECT_SOURCE_DIR}/Russian/project.mwz)

