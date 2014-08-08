

# configure abi checker
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/abi/abi_checker_last.xml.in ${CMAKE_CURRENT_BINARY_DIR}/abi/abi_checker_last.xml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/abi/check_abi.sh.in ${CMAKE_CURRENT_BINARY_DIR}/abi/check_abi.sh)
add_custom_target(abi-check
  "${CMAKE_CURRENT_BINARY_DIR}/abi/check_abi.sh")



