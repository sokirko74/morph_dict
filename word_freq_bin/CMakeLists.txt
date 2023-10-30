declare_cmake_min_version()

project(word_freq_bin)

add_executable (word_freq_bin word_freq_bin.cpp)

target_link_libraries(word_freq_bin
    lemmatizer_base_lib
)
