declare_cmake_min_version()

project(slf_convert)

add_executable (${PROJECT_NAME}
        slf_convert.cpp
)

target_link_libraries(${PROJECT_NAME} 
    morph_wizard_lib
)
