# main file for command line tools
#

## davix tool lib
add_library(davix_tool_lib STATIC
  davix_tool_params.cpp
  davix_tool_util.cpp
  "${SRC_SIMPLE_GET_PASS}"
  davix_op.cpp
  davix_taskqueue.cpp
  davix_thread.cpp
  davix_thread_pool.cpp
  davix_config_parser.cpp
)

target_include_directories(davix_tool_lib PUBLIC
  ${DAVIX_INTERNAL_INCLUDES}
)


## davix cmd line
add_executable(davix_tool davix_tool_main.cpp)

target_link_libraries(davix_tool davix_tool_lib libdavix)
set_target_properties(davix_tool PROPERTIES
                                OUTPUT_NAME "davix-http")

## davix ls cmd line
add_executable(davix_ls_tool davix_tool_ls_main.cpp)

target_link_libraries(davix_ls_tool davix_tool_lib libdavix ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(davix_ls_tool PROPERTIES
                                OUTPUT_NAME "davix-ls")


## davix get cmd line
add_executable(davix_get_tool davix_tool_get_main.cpp)

target_link_libraries(davix_get_tool davix_tool_lib libdavix pthread ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(davix_get_tool PROPERTIES
                                OUTPUT_NAME "davix-get")

## davix put cmd line
add_executable(davix_put_tool davix_tool_put_main.cpp)

target_link_libraries(davix_put_tool davix_tool_lib libdavix pthread ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(davix_put_tool PROPERTIES
                                OUTPUT_NAME "davix-put")


## davix del cmd line
add_executable(davix_mkdir_tool davix_tool_mkcol_main.cpp)

target_link_libraries(davix_mkdir_tool davix_tool_lib libdavix)
set_target_properties(davix_mkdir_tool PROPERTIES
                                OUTPUT_NAME "davix-mkdir")

## davix rm cmd line
add_executable(davix_rm_tool davix_tool_rm_main.cpp)

target_link_libraries(davix_rm_tool davix_tool_lib libdavix ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(davix_rm_tool PROPERTIES
                                OUTPUT_NAME "davix-rm")

## davix mv cmd line
add_executable(davix_mv_tool davix_tool_mv_main.cpp)

target_link_libraries(davix_mv_tool davix_tool_lib libdavix)
set_target_properties(davix_mv_tool PROPERTIES
                                OUTPUT_NAME "davix-mv")



install(TARGETS davix_tool davix_ls_tool davix_get_tool davix_put_tool davix_mkdir_tool davix_rm_tool davix_mv_tool
         DESTINATION ${BIN_INSTALL_DIR}/)

## davix cp cmd line
if (ENABLE_THIRD_PARTY_COPY)

    add_executable(davix_cp_tool davix_tool_copy_main.cpp)

    target_link_libraries(davix_cp_tool davix_copy davix_tool_lib libdavix)
    set_target_properties(davix_cp_tool PROPERTIES
                                OUTPUT_NAME "davix-cp")

    install(TARGETS davix_cp_tool
            DESTINATION ${BIN_INSTALL_DIR}/)

endif (ENABLE_THIRD_PARTY_COPY)
