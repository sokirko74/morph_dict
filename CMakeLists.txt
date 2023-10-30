macro (declare_cmake_min_version)
    cmake_minimum_required(VERSION 3.16)
endmacro(declare_cmake_min_version)

declare_cmake_min_version()

project(morphology)


set (CMAKE_CXX_STANDARD 17)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")


add_subdirectory (contrib/iconv_subset)
add_subdirectory (common)
add_subdirectory (agramtab)
add_subdirectory (morph_wizard)
add_subdirectory (lemmatizer_base_lib)
add_subdirectory (morph_gen)
add_subdirectory (homon_freq_bin)
add_subdirectory (word_freq_bin)
add_subdirectory (data)
add_subdirectory (test_lem)
