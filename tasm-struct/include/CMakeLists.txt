cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
ExternalProject_Add(expat
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/expat-2.2.0
  CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/expat-2.2.0/configure --prefix=<INSTALL_DIR>
  BUILD_COMMAND ${MAKE}
)