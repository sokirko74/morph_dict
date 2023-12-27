declare_cmake_min_version()

project(test_morph_wizard)

add_executable (${PROJECT_NAME} test_morph_wizard.cpp )

target_link_libraries(${PROJECT_NAME} 
    morph_wizard_lib
)


add_test(
        NAME MorpWizardTest 
        COMMAND test_morph_wizard
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

