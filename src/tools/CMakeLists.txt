# main file for command line tools
#




LIST(APPEND davix_main_src "davix_tool_main.cpp")
LIST(APPEND davix_ls_main_src "davix_tool_ls_main.cpp")
LIST(APPEND davix_get_main_src "davix_tool_get_main.cpp")
LIST(APPEND davix_put_main_src "davix_tool_put_main.cpp")
LIST(APPEND davix_del_main_src "davix_tool_del_main.cpp")
LIST(APPEND davix_tool_common_src "davix_tool_params.cpp" "davix_tool_util.cpp")

link_directories(${PROJECT_BINARY_DIR}/src/)

## davix cmd line
add_executable(davix_tool ${davix_main_src} ${davix_tool_common_src} ${SRC_SIMPLE_GET_PASS})

target_link_libraries(davix_tool davix)
set_target_properties(davix_tool PROPERTIES
                                OUTPUT_NAME "davix")
add_dependencies(davix_tool libdavix)

## davix ls cmd line
add_executable(davix_ls_tool ${davix_ls_main_src} ${davix_tool_common_src} ${SRC_SIMPLE_GET_PASS})

target_link_libraries(davix_ls_tool davix)
set_target_properties(davix_ls_tool PROPERTIES
                                OUTPUT_NAME "davix-ls")
add_dependencies(davix_ls_tool libdavix)


## davix get cmd line
add_executable(davix_get_tool ${davix_get_main_src} ${davix_tool_common_src} ${SRC_SIMPLE_GET_PASS})

target_link_libraries(davix_get_tool davix)
set_target_properties(davix_get_tool PROPERTIES
                                OUTPUT_NAME "davix-get")
add_dependencies(davix_get_tool libdavix)

## davix put cmd line
add_executable(davix_put_tool ${davix_put_main_src} ${davix_tool_common_src} ${SRC_SIMPLE_GET_PASS})

target_link_libraries(davix_put_tool davix)
set_target_properties(davix_put_tool PROPERTIES
                                OUTPUT_NAME "davix-put")
add_dependencies(davix_put_tool libdavix)


## davix del cmd line
add_executable(davix_del_tool ${davix_del_main_src} ${davix_tool_common_src} ${SRC_SIMPLE_GET_PASS})

target_link_libraries(davix_del_tool davix)
set_target_properties(davix_del_tool PROPERTIES
                                OUTPUT_NAME "davix-del")
add_dependencies(davix_del_tool libdavix)


install(TARGETS davix_tool davix_ls_tool davix_get_tool davix_put_tool davix_del_tool
         DESTINATION ${BIN_INSTALL_DIR}/)
