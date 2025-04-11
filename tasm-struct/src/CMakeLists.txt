cmake_minimum_required(VERSION 3.5)

ExternalProject_Get_Property(expat INSTALL_DIR)

# compiler flags
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -Wall -O3 -std=c++11 -g"
)

# include boost and path of CMakeLists.txt
include_directories(
  ${Boost_INCLUDE_DIR}
  ${CMAKE_BINARY_DIR}
  common
  data_structures
  data_structures/structure_search
  data_structures/tasm
  interfaces/tasm
  nodes
  parser
  parser
  rted
  sed
  structure_search
  tasm
  upper_bound
  visualisation
  wrappers
  zhang_shasha
)

# the set of all source files
set(tree_similarity_sources
  tree_similarity.cc
  common/timing.cc
)

add_custom_target(${PROJECT_NAME}-dep)

add_library(expat-a STATIC IMPORTED)
set_property(TARGET expat-a PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/libexpat.a)
add_dependencies(${PROJECT_NAME}-dep expat)

# uncomment if Boost is used
#find_package(Boost 1.55 REQUIRED)
#target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

# Remove this line that's causing problems
# list(APPEND tree_similarity_sources ${INSTALL_DIR}/include/expat.h)

# build executable
add_executable(${PROJECT_NAME} ${tree_similarity_sources})

include_directories(${INSTALL_DIR}/include/)
target_link_libraries(${PROJECT_NAME} expat-a)


# OLD:
# exec error:   "./tree_similarity" "'{a}'" "'{a{b}{c}}'"
# "-"           "./tree_similarity" "'{a{b{g}}{c{f}}{del}}'" "'{a{b{g}}{c{f}}}'"
# "-"           "./tree_similarity" "'{a{b{g}}{del}{c{f}}}'" "'{a{b{g}}{c{f}}}'"
# "-"           "./tree_similarity" "'{a{del}{b{g}}{c{f}}}'" "'{a{b{g}}{c{f}}}'"
# "-"           "./tree_similarity" "'{a{b{g{del}}}{c{f}}}'" "'{a{b{g}}{c{f}}}'"
# "-"           "./tree_similarity" "'{a{b{del{g}}}{c{f}}}'" "'{a{b{g}}{c{f}}}'"
#-------------------------------------------------------------------------------
# works:        "./tree_similarity" "'{a{b{g}}{c{f}}}'" "'{a{c{f}{g}}{d}}'"

# set test inputs (from stdin)
set(TEST_TREE_STDIN_001 "'{a{b{c}{d{e}{f}}}{x}}}'")
set(TEST_TREE_STDIN_002 "'{b{c}{d{e}{f}}}'")

add_custom_target(execute
  "${PROJECT_NAME_EXEC}" "${TEST_TREE_STDIN_001}" "${TEST_TREE_STDIN_002}"
)

# dynamic code analysis - valgrind
find_program(CHECK_DYNAMIC_COMMAND valgrind)
add_custom_target(check-dynamic "${CHECK_DYNAMIC_COMMAND}"
  "-v"
  "--leak-check=full"
  "--show-reachable=yes"
  "--track-origins=yes"
  "--trace-children=yes"
  "${PROJECT_NAME_EXEC}"
  "${TEST_TREE_STDIN_001}"
  "${TEST_TREE_STDIN_002}"
)

# static code analysis - cppcheck
find_program(CHECK_STATIC_COMMAND cppcheck)
add_custom_target(check-static "${CHECK_STATIC_COMMAND}"
  "--enable=all"
  "--suppress=missingIncludeSystem"
  "${PROJECT_SOURCE_DIR}"
)

# static and dynamic code analysis
add_custom_target(check DEPENDS check-static check-dynamic)
