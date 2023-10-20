declare_cmake_min_version()

project(LemmatizerBaseLib)

add_library(LemmatizerBaseLib
        CreatePredictionBase.cpp
        Lemmatizers.cpp
        MorphAutomBuilder.cpp
        MorphAutomat.cpp
        MorphDict.cpp
        MorphDictBuilder.cpp
        Paradigm.cpp
        Predict.cpp
        Statistic.cpp
        MorphanHolder.cpp
        rus_numerals.cpp
        FuzzySearch.cpp
        common_consts.h
        LemmaInfoSerialize.h
        Lemmatizers.h
        MorphanHolder.h
        MorphAutomat.h
        MorphAutomBuilder.h
        MorphDictBuilder.h
        MorphDict.h
        Paradigm.h
        Predict.h
        rus_numerals.h
        StaticVectorMap.h
        Statistic.h

)

target_link_libraries(LemmatizerBaseLib
        AgramtabLib
        MorphWizardLib
        )

