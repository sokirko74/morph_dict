declare_cmake_min_version()

project(MorphWizardLib)

add_library(MorphWizardLib
        wizard.cpp
        FormInfo.cpp
        DumpParadigm.cpp
        DumpParadigm.h
 "LemmaPredict.cpp" "LemmaPredict.h")

target_link_libraries(MorphWizardLib
    AgramtabLib
)

