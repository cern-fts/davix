


file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build/html/")
addDoxyGeneration("Doxyfile.in")
SET( l_doc "${CMAKE_CURRENT_BINARY_DIR}/build/html/")

install(DIRECTORY ${l_doc}
		DESTINATION ${DOC_INSTALL_DIR}/html/) # install examples 




