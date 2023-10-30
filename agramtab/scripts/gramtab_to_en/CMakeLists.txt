project(gramtab_to_en)

add_executable (gramtab_to_en
        gramtab_to_en.cpp 
)

target_link_libraries(gramtab_to_en
    AgramtabLib
    morph_wizard_lib
)

