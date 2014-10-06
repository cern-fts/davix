# main file for command line tools
#




LIST(APPEND davix_main_src "davix_tool_main.cpp")
LIST(APPEND davix_ls_main_src "davix_tool_ls_main.cpp")
LIST(APPEND davix_get_main_src "davix_tool_get_main.cpp")
LIST(APPEND davix_put_main_src "davix_tool_put_main.cpp")
LIST(APPEND davix_rm_main_src "davix_tool_rm_main.cpp")
LIST(APPEND davix_mkcol_main_src "davix_tool_mkcol_main.cpp")
LIST(APPEND davix_copy_main_src "davix_tool_copy_main.cpp")
LIST(APPEND davix_tool_common_src "${CMAKE_CURRENT_SOURCE_DIR}/davix_tool_params.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/davix_tool_util.cpp" "${SRC_SIMPLE_GET_PASS}")
SET(davix_tool_common_src_up "${davix_tool_common_src}" PARENT_SCOPE)

link_directories(${PROJECT_BINARY_DIR}/src/)


add_definitions(-DNO_BOOST_INCLUDE)

## davix tool lib
add_library(davix_tool_lib STATIC ${davix_tool_common_src})


## davix cmd line
add_executable(davix_tool ${davix_main_src})

target_link_libraries(davix_tool davix_tool_lib davix)
set_target_properties(davix_tool PROPERTIES
                                OUTPUT_NAME "davix-http")
add_dependencies(davix_tool libdavix)

## davix ls cmd line
add_executable(davix_ls_tool ${davix_ls_main_src})

target_link_libraries(davix_ls_tool davix_tool_lib davix)
set_target_properties(davix_ls_tool PROPERTIES
                                OUTPUT_NAME "davix-ls")
add_dependencies(davix_ls_tool libdavix)


## davix get cmd line
add_executable(davix_get_tool ${davix_get_main_src})

target_link_libraries(davix_get_tool davix_tool_lib davix)
set_target_properties(davix_get_tool PROPERTIES
                                OUTPUT_NAME "davix-get")
add_dependencies(davix_get_tool libdavix)

## davix put cmd line
add_executable(davix_put_tool ${davix_put_main_src})

target_link_libraries(davix_put_tool davix_tool_lib davix)
set_target_properties(davix_put_tool PROPERTIES
                                OUTPUT_NAME "davix-put")
add_dependencies(davix_put_tool libdavix)


## davix del cmd line
add_executable(davix_mkdir_tool ${davix_mkcol_main_src})

target_link_libraries(davix_mkdir_tool davix_tool_lib davix)
set_target_properties(davix_mkdir_tool PROPERTIES
                                OUTPUT_NAME "davix-mkdir")
add_dependencies(davix_mkdir_tool libdavix)

## davix rm cmd line
add_executable(davix_rm_tool ${davix_rm_main_src})

target_link_libraries(davix_rm_tool davix_tool_lib davix)
set_target_properties(davix_rm_tool PROPERTIES
                                OUTPUT_NAME "davix-rm")
add_dependencies(davix_rm_tool libdavix)


install(TARGETS davix_tool davix_ls_tool davix_get_tool davix_put_tool davix_mkdir_tool davix_rm_tool
         DESTINATION ${BIN_INSTALL_DIR}/)

## davix cp cmd line
if (ENABLE_THIRD_PARTY_COPY)

    add_executable(davix_cp_tool ${davix_copy_main_src})

    target_link_libraries(davix_cp_tool davix_copy davix_tool_lib)
    set_target_properties(davix_cp_tool PROPERTIES
                                OUTPUT_NAME "davix-cp")
    add_dependencies(davix_cp_tool davix_copy)
    
    install(TARGETS davix_cp_tool
            DESTINATION ${BIN_INSTALL_DIR}/)
    
endif (ENABLE_THIRD_PARTY_COPY)

