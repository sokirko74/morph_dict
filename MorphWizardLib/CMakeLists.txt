declare_cmake_min_version()

project(MorphWizardLib)

add_library(MorphWizardLib
        wizard.cpp
        "wizard_base.cpp"
        DumpParadigm.cpp
        DumpParadigm.h
        LemmaPredict.cpp
        DumpParadigm.h
        "wizard_base.h"
        LemmaPredict.h
        OperationMeter.h
        wizard.h
        flexia_model.cpp
        "lemma_info.h" 
        "lemma_info.cpp" "flexia_model.h" "paradigm_info.cpp" "paradigm_info.h" "accent_model.cpp" "accent_model.h")

target_link_libraries(MorphWizardLib
    AgramtabLib
)

add_subdirectory(scripts/read_mrd)
