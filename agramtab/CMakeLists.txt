declare_cmake_min_version()

add_library(AgramtabLib
        agramtab.cpp
        agramtab.h
        EngGramTab.cpp
        EngGramTab.h
        RusGramTab.cpp
        RusGramTab.h
        GerGramTab.cpp
        GerGramTab.h

        GramInfo.h
 )

target_link_libraries(AgramtabLib
        morphology_common
)

add_subdirectory (tests/CheckGramTab)