# CMakeLists.txt of executable balise_codec

add_executable(balise_codec)

target_sources(balise_codec 
    PRIVATE
        main.cpp
)

target_link_libraries(balise_codec
    PRIVATE
        ss36
)