declare_cmake_min_version()

project(lemmatizer_base_lib)

add_library(lemmatizer_base_lib
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

target_link_libraries(lemmatizer_base_lib
        AgramtabLib
        morph_wizard_lib
        )

