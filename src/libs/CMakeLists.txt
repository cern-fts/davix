# libs CMakefile for third party deps


add_subdirectory(containers)
add_subdirectory(security)


SET(src_vector_chunk ${src_vector_chunk} PARENT_SCOPE)
SET(src_crypto ${src_crypto} PARENT_SCOPE)
