declare_cmake_min_version()

add_library(AgramtabLib
        EngGramTab.cpp
        RusGramTab.cpp
        GerGramTab.cpp
        agramtab_.cpp
        AncodePattern.cpp
        agramtab_.h
        AncodePattern.h
        eng_consts.h
        EngGramTab.h
        ger_consts.h
        GerGramTab.h
        GramInfo.h
        rus_consts.h
        RusGramTab.h

)

target_link_libraries(AgramtabLib
        morphology_common
)

add_subdirectory (tests/CheckGramTab)
add_subdirectory (scripts/gramtab_to_json)
