# libs CMakefile for third party deps


add_subdirectory(adevpp)
add_subdirectory(datetime)





SET(src_vector_chunk ${src_vector_chunk} PARENT_SCOPE)
SET(src_crypto ${src_crypto} PARENT_SCOPE)
SET(src_datetime ${src_datetime} PARENT_SCOPE)
