declare_cmake_min_version()

project(StatDatBin)

add_executable (StatDatBin homon_freq_bin.cpp)

target_link_libraries(StatDatBin 
    lemmatizer_base_lib
)
