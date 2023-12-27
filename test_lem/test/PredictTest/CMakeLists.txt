declare_cmake_min_version()

project(test_predict)

add_executable (${PROJECT_NAME} "test_predict.cpp")

target_link_libraries(${PROJECT_NAME}
        lemmatizer_base_lib
)

add_test(
    NAME PredictTestRussian 
    COMMAND ${PROJECT_NAME} russian.txt 0.65
    WORKING_DIRECTORY  ${PROJECT_SOURCE_DIR}
)

