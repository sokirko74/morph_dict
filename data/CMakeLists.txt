set(CMAKE_LEGACY_CYGWIN_WIN32 0)
declare_cmake_min_version()

project(MorphAllLanguages)

if  ( DEFINED DICTS_FOLDER)
   set( output_morph_folder  ${DICTS_FOLDER}/Morph )
else()
    set( output_morph_folder  ${PROJECT_SOURCE_DIR}/../../Dicts/Morph )
endif()

message("dicts_folder=${output_morph_folder}")

macro (RunMorphGen langua)
    set( input_folder  ${PROJECT_SOURCE_DIR}/${langua} )
    set( output_folder  ${output_morph_folder}/${langua} )
    make_directory(${output_folder})

    add_custom_target(
        "MorphGen_on_${langua}"
         SOURCES ${output_folder}/morph.bases ${output_folder}/morph.annot ${output_folder}/morph.forms_autom ${output_folder}/gramtab.tab
    )

    add_custom_command(
            OUTPUT ${output_folder}/gramtab.tab PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${input_folder}/gramtab.tab ${output_folder}/gramtab.tab
            VERBATIM
    )
    add_custom_command(
        OUTPUT ${output_folder}/morph.bases ${output_folder}/morph.annot ${output_folder}/morph.forms_autom
        COMMAND MorphGen --input project.mwz --output-folder ${output_folder} --postfix-len 5 --min-freq 3
        DEPENDS MorphGen ${input_folder}/morphs.mrd ${input_folder}/project.mwz ${input_folder}/gramtab.tab
        COMMENT "Convert Morphology ${langua}"                 
        WORKING_DIRECTORY ${input_folder}
        VERBATIM
    )
endmacro()


macro (StatDatBinRun langua Topic)
    set (src ${PROJECT_SOURCE_DIR}/${langua}/StatData.txt)
    set (morph_folder ${output_morph_folder}/${langua})
    set (trg ${morph_folder}/${Topic}homoweight.bin)
    add_custom_command(
        OUTPUT ${trg}
        COMMAND StatDatBin --morph-folder ${morph_folder} --input ${src} --language ${langua} --output ${trg}
        DEPENDS StatDatBin ${src} "${morph_folder}/morph.bases" "${morph_folder}/gramtab.tab"
        COMMENT "StatDatBin"
        VERBATIM
    )
endmacro()

macro (WordDatBinRun langua Topic)
    set (src ${PROJECT_SOURCE_DIR}/${langua}/WordData.txt)
    set (morph_folder ${output_morph_folder}/${langua})
    set (trg ${morph_folder}/${Topic}wordweight.bin)
    add_custom_command(
        OUTPUT ${trg}
        COMMAND WordDatBin --morph-folder ${morph_folder} --input ${src} --language ${langua} --output ${trg}
        DEPENDS WordDatBin ${src} "${morph_folder}/morph.bases" "${morph_folder}/gramtab.tab"
        COMMENT "WordDatBin"
        VERBATIM
    )
endmacro()


macro (CreateAllMorphBinFiles langua)
    RunMorphGen (${langua})
    
    foreach (Topic l c f)
        StatDatBinRun ( ${langua} ${Topic} ) 
        WordDatBinRun ( ${langua} ${Topic} )
    endforeach ()

    set( folder  ${output_morph_folder}/${langua} )
    add_custom_target(
        "${langua}_Morph"
        SOURCES ${folder}/morph.bases ${folder}/chomoweight.bin ${folder}/lhomoweight.bin ${folder}/fhomoweight.bin ${folder}/cwordweight.bin ${folder}/lwordweight.bin ${folder}/fwordweight.bin
        WORKING_DIRECTORY ${folder}
    )
endmacro()

CreateAllMorphBinFiles(Russian)
CreateAllMorphBinFiles(German)
CreateAllMorphBinFiles(English)

add_custom_target(
    MorphDicts
    DEPENDS  Russian_Morph German_Morph English_Morph
)

