declare_cmake_min_version()

set(my_SOURCES
        ../lemmatizer_base_lib/rus_numerals.cpp
        utilit.cpp
        argparse.h
        base_types.h
        bserialize.h
        json.h
        single_byte_encoding.h
        single_byte_encoding.cpp
        util_classes.h
        utilit.h
        json.cpp
        utf8.cpp
        rapidjson.cpp
        rapidjson.h
)

add_library(morphology_common  ${my_SOURCES})

add_subdirectory(tests)


target_link_libraries(morphology_common
    iconv_subset ZLIB::ZLIB
)


