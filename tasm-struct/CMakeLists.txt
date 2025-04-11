
cmake_minimum_required(VERSION 3.0)
project(tasm-struct)

# Remove or modify the line that sets -stdlib=libc++
# If there's a line like:
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
# Change it to check if the compiler is Clang first:
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

set(PROJECT_NAME_EXEC "./${PROJECT_NAME}")
set(EXECUTABLE_OUTPUT_PATH ..)
add_subdirectory(include/)
add_subdirectory(src/)