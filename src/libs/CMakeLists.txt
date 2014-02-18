# libs CMakefile for third party deps


add_subdirectory(adevpp)
add_subdirectory(lockers)
add_subdirectory(logger)





SET(src_vector_chunk ${src_vector_chunk} PARENT_SCOPE)
SET(src_crypto ${src_crypto} PARENT_SCOPE)
SET(src_lockers ${src_lockers} PARENT_SCOPE)
SET(src_logger ${src_logger} PARENT_SCOPE)
