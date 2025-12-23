message ("Parsing: CMakeLists.txt of Python Module")

add_library(py_balise_codec SHARED)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
set_target_properties(py_balise_codec 
    PROPERTIES
        VERSION 9.0     # Uses project version
        SOVERSION 2     # API version (increment when breaking changes occur)
)
endif ()

# tbd: add versioning

IF (CMAKE_SYSTEM_NAME STREQUAL "Windows")
target_sources(py_balise_codec 
    PUBLIC
        python_api.cpp
#        ${CMAKE_SOURCE_DIR}\\version.rc
)
ENDIF ()

IF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
target_sources(py_balise_codec 
    PUBLIC
        python_api.cpp
#        ${CMAKE_SOURCE_DIR}/version.h
)
ENDIF ()

target_link_libraries(py_balise_codec
    PRIVATE
        ss36
)
