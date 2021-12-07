macro (declare_cmake_min_version)
    cmake_minimum_required(VERSION 3.16)
endmacro(declare_cmake_min_version)

declare_cmake_min_version()

project(morphology)


set (CMAKE_CXX_STANDARD 17)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")


add_subdirectory (contrib/iconv_subset)
add_subdirectory (common)
add_subdirectory (AgramtabLib)
add_subdirectory (MorphWizardLib)
add_subdirectory (LemmatizerBaseLib)
add_subdirectory (MorphGen)
add_subdirectory (StatDatBin)
add_subdirectory (WordDatBin)
add_subdirectory (data)
add_subdirectory (TestLem)
