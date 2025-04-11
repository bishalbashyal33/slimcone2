cmake_minimum_required(VERSION 3.5)

  # Prefer 3.0+ for better Clang support

# Set Clang as the compiler
set(CMAKE_CXX_COMPILER "clang++")

# Use libc++ (LLVM's C++ standard library) instead of libstdc++
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall -Wextra -pedantic-errors -O3")

# Link against libc++ and libc++abi
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")

# Include Boost and other directories
include_directories(
  ${Boost_INCLUDE_DIR}
  ${CMAKE_BINARY_DIR}
  parser
  nodes
  data_structures
  data_structures/tasm
  tasm
  interfaces/tasm
  common
  wrappers
  index
  filter
  distance
)

# Source files
set(topk_queries_sources
  ${PROJECT_NAME}.cc
  parser/abstract_event_handler.h
  parser/default_event_handler.h
  parser/xml_parser.h
  parser/label_set_event_handler.h
  parser/label_set_index_event_handler.h
  parser/postorder_queue_event_handler.h
  nodes/node.h
  nodes/string_node_data.h
  data_structures/tasm/tasm_tree.h
  data_structures/array_2d.h
  data_structures/label_hash_map.h
  data_structures/k_heap.h
  data_structures/vector_multimap.h
  tasm/tasm_dynamic.h
  interfaces/tasm/postorder_queue_interface.h
  interfaces/tasm/postorder_source_interface.h
  common/hash_functors.h
  common/statistics.h
  common/log.h
  common/timing.h
  common/timing.cc
  wrappers/node_distance_pair.h
  index/abstract_index.h
  index/abstract_inverted_list_index.h
  index/abstract_inverted_map_index.h
  index/inverted_list_index.h
  index/slim_inverted_list_index.h
  index/slim_inverted_map_index.h
  filter/label_overlap_filter.h
  filter/cone_filter.h
  filter/shin_cone_filter.h
  filter/updatable_shin_cone_filter.h
  filter/lower_bound_merge_filter.h
  distance/tasmted.h
)

add_custom_target(${PROJECT_NAME}-dep)

add_library(expat-a STATIC IMPORTED)
set_property(TARGET expat-a PROPERTY IMPORTED_LOCATION "/home/azureuser/kocher-sigmod2019-reproducibility/slim/build/include/expat-prefix/lib/libexpat.a")
add_dependencies(${PROJECT_NAME}-dep expat)

# Build executable
add_executable(${PROJECT_NAME} ${topk_queries_sources})

include_directories(${INSTALL_DIR}/include/)
target_link_libraries(${PROJECT_NAME} expat-a c++ c++abi)  # Link libc++ and libc++abi

# Dynamic analysis (valgrind)
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

# Static analysis (cppcheck)
find_program(CHECK_STATIC_COMMAND cppcheck)
add_custom_target(check-static "${CHECK_STATIC_COMMAND}"
  "--enable=all"
  "--suppress=missingIncludeSystem"
  "${PROJECT_SOURCE_DIR}"
)

add_custom_target(check DEPENDS check-static check-dynamic)
