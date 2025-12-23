# CMakeLists.txt of executable tester

add_executable(tester)

target_sources(tester PRIVATE
    tester.cpp
)

target_link_libraries(tester
    PRIVATE
        ss36
)