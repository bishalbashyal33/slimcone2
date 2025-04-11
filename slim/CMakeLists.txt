cmake_minimum_required(VERSION 3.5)

project(topk_queries)
set(PROJECT_NAME_EXEC "./${PROJECT_NAME}")

set(EXECUTABLE_OUTPUT_PATH ..)


add_subdirectory(include/)
add_subdirectory(src/)
