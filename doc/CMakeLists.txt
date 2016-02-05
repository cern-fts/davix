


file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build/html/")

if(ENABLE_HTML_DOCS)
  addDoxyGeneration("Doxyfile.in")
  SET( l_doc "${CMAKE_CURRENT_BINARY_DIR}/build/html/")

  install(DIRECTORY ${l_doc}
    DESTINATION ${DOC_INSTALL_DIR}/html/doxygen) # install examples

  find_package(Sphinx)

  if(SPHINX_FOUND)
    add_custom_target(sphinx
      DEPENDS doc
      COMMAND ${SPHINX_EXECUTABLE}
      -q -b html
      "${CMAKE_CURRENT_SOURCE_DIR}/sphinx"
      "${CMAKE_CURRENT_BINARY_DIR}/build/sphinx")

    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build/sphinx"
     DESTINATION ${DOC_INSTALL_DIR}/html/sphinx) # install examples
  endif(SPHINX_FOUND)
endif(ENABLE_HTML_DOCS)

FILE(GLOB man1_page ${CMAKE_CURRENT_SOURCE_DIR}/man/*.1)
install(FILES ${man1_page}
                        DESTINATION ${MAN_INSTALL_DIR}/man1/)

FILE(GLOB man3_page ${CMAKE_CURRENT_SOURCE_DIR}/man/*.3)
install(FILES ${man3_page}
                        DESTINATION ${MAN_INSTALL_DIR}/man3/)


