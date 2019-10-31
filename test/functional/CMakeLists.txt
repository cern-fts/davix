# main file for src


set(src_davix_test_lib "davix_test_lib.cpp")

#set( src_test_req "test_request.cpp")
#set( src_test_checksum "test_checksum.cpp")
#set( src_test_stat_session "test_stat_session.cpp")
#set( src_test_rw "davix_rw_test.cpp")
#set( src_test_opendir "test_opendir.cpp")
#set( src_test_opendir_partial "test_opendir_partial.cpp")
#set( src_test_opendirpp "test_opendirpp.cpp")
#set( src_test_directory "test_directory.cpp")
#set( src_test_auto_stat_mkdir_check "test_auto_stat_mkdir_check.cpp")
#set( src_test_auto_rmdir_unlink_delete "test_auto_rmdir_unlink_delete.cpp")
#set( src_test_valid_read_generic_simple "test_valid_read_generic_simple.cpp")
#set( src_test_valid_write_read_generic_simple "test_valid_write_read_generic_simple.cpp")
#set( src_test_valid_write_read_fd "test_valid_write_read_fd.cpp")
#set( src_test_valid_write_read_vector_simple "test_valid_write_read_vector_simple.cpp")
#set( src_tool_read_vector_simple "tool_read_vector_simple.cpp")
#set( src_tool_get_replicas "tool_get_replica_simple.cpp")
#set( src_test_mv "test_mv.cpp")

add_library(test_davix_functional_lib STATIC ${src_davix_test_lib})

#add_executable(test_request ${src_test_req} )
#target_link_libraries(test_request libdavix  test_davix_functional_lib)

#add_executable(tool_read_vector_simple ${src_tool_read_vector_simple} )
#target_link_libraries(tool_read_vector_simple libdavix test_davix_functional_lib )

#add_executable(test_rw_fd ${src_test_valid_write_read_fd})
#target_link_libraries(test_rw_fd libdavix test_davix_functional_lib )

#add_executable(test_stat_auto_check ${src_test_auto_stat_mkdir_check})
#target_link_libraries(test_stat_auto_check libdavix test_davix_functional_lib)

#add_executable(test_checksum ${src_test_checksum})
#target_link_libraries(test_checksum libdavix test_davix_functional_lib)

#add_executable(test_auto_rmdir_unlink_delete ${src_test_auto_rmdir_unlink_delete})
#target_link_libraries(test_auto_rmdir_unlink_delete libdavix test_davix_functional_lib)

#add_executable(test_stat_session ${src_test_stat_session})
#target_link_libraries(test_stat_session libdavix test_davix_functional_lib)


#add_executable(test_rw ${src_test_rw})
#target_link_libraries(test_rw libdavix test_davix_functional_lib)

#add_executable(test_opendir ${src_test_opendir} )
#target_link_libraries(test_opendir libdavix test_davix_functional_lib)


#add_executable(test_directory ${src_test_directory} )
#target_link_libraries(test_directory libdavix test_davix_functional_lib)

#add_executable(test_opendir_partial ${src_test_opendir_partial})
#target_link_libraries(test_opendir_partial libdavix test_davix_functional_lib)

#add_executable(test_opendirpp ${src_test_opendirpp})
#target_link_libraries(test_opendirpp libdavix test_davix_functional_lib)

#add_executable(test_valid_read_generic_simple ${src_test_valid_read_generic_simple})
#target_link_libraries(test_valid_read_generic_simple libdavix test_davix_functional_lib)

#add_executable(test_valid_write_read_generic_simple ${src_test_valid_write_read_generic_simple})
#target_link_libraries(test_valid_write_read_generic_simple libdavix test_davix_functional_lib)

#add_executable(test_valid_write_read_vector_simple ${src_test_valid_write_read_vector_simple})
#target_link_libraries(test_valid_write_read_vector_simple libdavix test_davix_functional_lib)


#add_executable(tool_get_replicas ${src_tool_get_replicas})
#target_link_libraries(tool_get_replicas libdavix test_davix_functional_lib)

#add_executable(test_mv ${src_test_mv})
#target_link_libraries(test_mv libdavix test_davix_functional_lib)

add_executable(davix-tester ../TestcaseHandler.cpp davix-tester.cpp)
target_link_libraries(davix-tester libdavix test_davix_functional_lib)

install(TARGETS davix-tester
  DESTINATION ${BIN_INSTALL_DIR}/)

### DEPRECATED, to remove soon

# function(test_dav_endpoint_ronly name cred)

#     add_test(test_stat_session_${name} test_stat_session ${cred} ${name} ${name} ${name} ${name} ${name} ${name} ${name})
#     add_test(test_opendir_${name} test_opendir ${name} ${cred})
#   #  add_test(test_opendir_c_${name} test_opendir_c ${name} ${cred})
#     add_test(test_opendirpp_${name} test_opendirpp ${name} ${cred})
#   #  add_test(test_stat_c_${name} test_stat_c ${name} ${cred})
# endfunction(test_dav_endpoint_ronly name cred)

# function(test_dav_endpoint_rw name cred)

#     add_test(test_stat_session_${name} test_stat_session ${cred} ${name} ${name} ${name} ${name} ${name} ${name} ${name})
#     add_test(test_opendir_${name} test_opendir ${name} ${cred})
#     add_test(test_opendirpp_${name} test_opendirpp ${name} ${cred})
#     add_test(test_stat_auto_check_${name} test_stat_auto_check ${name} ${cred})
# endfunction(test_dav_endpoint_rw name cred)

# function(listdir_partial name size cred)
#     add_test(test_opendir_partial_${name}  test_opendir_partial ${name} ${size} ${cred})
# endfunction(listdir_partial name size cred)


# function(test_collection name cred)
#     add_test(test_directory_${name} test_directory ${name} ${cred})
# endfunction(test_collection name cred)

# function(test_valid_read_generic url cred)
#     add_test(test_valid_read_generic_${url} test_valid_read_generic_simple ${url} ${cred})
# endfunction(test_valid_read_generic url cred)

# function(test_valid_delete_all url cred)
#     add_test(test_auto_rmdir_unlink_delete_${url} test_auto_rmdir_unlink_delete ${url} ${cred})
# endfunction(test_valid_delete_all url cred)


# function(test_valid_write_read_generic url cred)
#     add_test(test_valid_write_read_generic_simple_${url} test_valid_write_read_generic_simple ${url} ${cred})
#     add_test(test_rw_fd_${url} test_rw_fd ${url} ${cred})
#     add_test(test_valid_write_read_vector_simple_${url} test_valid_write_read_vector_simple ${url} ${cred})
# endfunction(test_valid_write_read_generic url cred)

# function(test_replica_listing_existing url cred)
#    add_test(tool_get_replicas_${url} tool_get_replicas ${url} ${cred})
# endfunction(test_replica_listing_existing url cred)

# function(test_rename url cred)
#     add_test(test_mv_${url} test_mv ${url} ${cred})
# endfunction(test_rename url cred)
