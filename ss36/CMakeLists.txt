message("Parsing: CMakeLists.txt of library ss36")

add_library (ss36)

# Set PIC for .so compilation on Linux
set_target_properties(ss36 
    PROPERTIES
        POSITION_INDEPENDENT_CODE TRUE
)

target_sources(ss36
    PRIVATE
        ansi_escapes.c
        balise_codec.cpp
        longnum.cpp
        parse_input.cpp
        telegram.cpp
        "lookup_tables.cpp"
        useful_functions.cpp

    PUBLIC
        FILE_SET HEADERS
        FILES
            ansi_escapes.h
            balise_codec.h
            BS_thread_pool.hpp
            CLI11.hpp
            colors.h
            longnum.h
            parse_input.h
            telegram.h
            "lookup_tables.h"
            useful_functions.h
            errcodes.h
 "temp.c")
