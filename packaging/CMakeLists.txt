# configure packaging files

configure_file (
               "${CMAKE_CURRENT_SOURCE_DIR}/rpm/specs/davix.spec.in"
               "${CMAKE_CURRENT_SOURCE_DIR}/rpm/specs/davix.spec"
               @ONLY
)





