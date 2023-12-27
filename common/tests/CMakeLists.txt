declare_cmake_min_version()

project(test_common)

add_executable (${PROJECT_NAME} test_common.cpp )

target_link_libraries(${PROJECT_NAME} 
    morphology_common
)

add_test(
        NAME TestCommon 
        COMMAND test_common
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

