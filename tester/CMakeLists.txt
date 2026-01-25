message ("Parsing: CMakeLists.txt of tester")

add_executable(tester)

target_sources(tester PRIVATE
    tester.cpp
 )

target_link_libraries(tester
    PRIVATE
        ss36
)

message ("Parsing tests in folder ${CMAKE_CURRENT_SOURCE_DIR}")
message ("Binary path is: ${CMAKE_BINARY_DIR}")

# test the library:
add_test (NAME test_00 COMMAND $<TARGET_FILE:tester>)

# test file i/o of the main program:
add_test (NAME test_01 COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/01/testscript.py $<TARGET_FILE:balise_codec>)

# test command line string:
add_test (NAME test_02 COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/02/testscript.py $<TARGET_FILE:balise_codec>)

# test python library:
add_test (NAME test_03 COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/03/testscript.py $<TARGET_FILE:py_balise_codec>)

# test calc_all in python library:
add_test (NAME test_04 COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/04/testscript.py $<TARGET_FILE:py_balise_codec>)
