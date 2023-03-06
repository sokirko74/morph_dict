declare_cmake_min_version()

project(PredictTest)

add_executable (${PROJECT_NAME} PredictTest.cpp)

target_link_libraries(${PROJECT_NAME}
        LemmatizerBaseLib
)

add_test(
    NAME PredictTestRussian 
    COMMAND ${PROJECT_NAME} russian.txt 0.65
    WORKING_DIRECTORY  ${PROJECT_SOURCE_DIR}
)

