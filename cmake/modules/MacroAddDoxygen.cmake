##
## Doxygen macro, allow Doxygen generation from cmake
## do a ""make doc" for the generation


macro(addDoxyGeneration DOXYFILE_LOCATION)


	find_package(Doxygen)
	if(DOXYGEN_FOUND)

	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE_LOCATION} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

	add_custom_target(doxygen
	${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Generating API documentation with Doxygen" VERBATIM
	)
	endif(DOXYGEN_FOUND)


endmacro(addDoxyGeneration DOXYFILE_LOCATION)
