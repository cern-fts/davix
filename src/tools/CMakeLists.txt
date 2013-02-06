# main file for command line tools
#




LIST(APPEND davix_main_src "davix_tool_main.cpp")
LIST(APPEND davix_tool_common_src "davix_tool_params.cpp" "davix_tool_util.cpp")

link_directories(${PROJECT_BINARY_DIR}/src/)
add_executable(davix_tool ${davix_main_src} ${davix_tool_common_src})

target_link_libraries(davix_tool davix)
set_target_properties(davix_tool PROPERTIES
                                OUTPUT_NAME "davix")
add_dependencies(davix_tool libdavix)


install(TARGETS davix_tool
         DESTINATION ${BIN_INSTALL_DIR}/)
