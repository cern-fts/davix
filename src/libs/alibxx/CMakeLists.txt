# libs CMakefile for A_LIB_NAMESPACE


SET(ALIBXX_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "ALIBXX src dir")


FILE(GLOB src_alibxx "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp")

add_definitions( -fPIC)
add_library(alibxx_core STATIC ${src_alibxx})

