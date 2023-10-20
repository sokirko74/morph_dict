declare_cmake_min_version()

project(MorphWizardLib)

add_library(MorphWizardLib
        wizard.cpp
        FormInfo.cpp
        DumpParadigm.cpp
        DumpParadigm.h
        LemmaPredict.cpp
        DumpParadigm.h
        FormInfo.h
        LemmaPredict.h
        OperationMeter.h
        wizard.h

)

target_link_libraries(MorphWizardLib
    AgramtabLib
)

