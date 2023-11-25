project(test_gramtab)

add_executable (${PROJECT_NAME}
  "test_gramtab.cpp"
)

target_link_libraries(${PROJECT_NAME} 
    AgramtabLib
)
add_test(
    NAME TestGramTab
    COMMAND ${PROJECT_NAME}
)

