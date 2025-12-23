message ("Parsing: CMakeLists.txt of main solution")

cmake_minimum_required (VERSION 3.10)
cmake_policy(SET CMP0091 NEW)   # needed for /MT

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)  
set(CMAKE_C_STANDARD_REQUIRED ON)

project(balise_codec LANGUAGES C CXX)

message ("CMAKE_HOST_SYSTEM_NAME=" ${CMAKE_HOST_SYSTEM_NAME})
message ("CMAKE_SYSTEM_NAME=" ${CMAKE_SYSTEM_NAME})
message ("CMAKE_SYSTEM=" ${CMAKE_SYSTEM})
message ("CMAKE_CURRENT_SOURCE_DIR=" ${CMAKE_CURRENT_SOURCE_DIR})

# set /MT instead of /MD:
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_subdirectory(balise_codec)
add_subdirectory(tester)
add_subdirectory(ss36)
add_subdirectory(py_balise_codec)
