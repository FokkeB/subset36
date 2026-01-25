message ("Parsing: CMakeLists.txt of Python Module")

add_library(py_balise_codec SHARED)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
set_target_properties(py_balise_codec 
    PROPERTIES
        VERSION 9.0     # Uses project version
        SOVERSION 2     # API version (increment when breaking changes occur)
)
endif ()

IF (CMAKE_SYSTEM_NAME STREQUAL "Windows")
target_sources(py_balise_codec 
    PRIVATE
        python_api.cpp

    PUBLIC
        FILE_SET HEADERS
        FILES
            python_api.h
)
ENDIF ()

IF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
target_sources(py_balise_codec 
    PUBLIC
        python_api.cpp
)
ENDIF ()

target_link_libraries(py_balise_codec
    PRIVATE
        ss36
)
