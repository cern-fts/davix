# main file for src


if(FUNCTIONAL_TESTS)

set(src_davix_test_lib "davix_test_lib.cpp")

#set( src_test_req "test_request.cpp")
#set( src_test_prop "test_propfind.cpp")
#set( src_test_stat "test_stat.cpp" )
#set( src_test_mkdir "test_mkdir.cpp")
#set( src_test_checksum "test_checksum.cpp")
#set( src_test_stat_session "test_stat_session.cpp")
#set( src_test_rw "davix_rw_test.cpp")
#set( src_test_opendir "test_opendir.cpp")
#set( src_test_opendir_partial "test_opendir_partial.cpp")
#set( src_test_opendirpp "test_opendirpp.cpp")
#set( src_test_listing "test_listing.cpp")
#set( src_test_directory "test_directory.cpp")
#set( src_test_auto_stat_mkdir_check "test_auto_stat_mkdir_check.cpp")
#set( src_test_auto_rmdir_unlink_delete "test_auto_rmdir_unlink_delete.cpp")
#set( src_test_valid_read_generic_simple "test_valid_read_generic_simple.cpp")
#set( src_test_valid_write_read_generic_simple "test_valid_write_read_generic_simple.cpp")
#set( src_test_valid_write_read_davfile "test_valid_write_read_davfile.cpp")
#set( src_test_valid_write_read_fd "test_valid_write_read_fd.cpp")
#set( src_test_valid_write_read_vector_simple "test_valid_write_read_vector_simple.cpp")
#set( src_tool_read_vector_simple "tool_read_vector_simple.cpp")
#set( src_tool_get_replicas "tool_get_replica_simple.cpp")
#set( src_test_mv "test_mv.cpp")

set( src_tester "tester.cpp")


add_library(test_davix_functional_lib STATIC ${src_davix_test_lib})

#add_executable(test_request ${src_test_req} )
#target_link_libraries(test_request libdavix  test_davix_functional_lib)

#add_executable(tool_read_vector_simple ${src_tool_read_vector_simple} )
#target_link_libraries(tool_read_vector_simple libdavix test_davix_functional_lib )

#add_executable(test_rw_fd ${src_test_valid_write_read_fd})
#target_link_libraries(test_rw_fd libdavix test_davix_functional_lib )

#add_executable(test_prop ${src_test_prop} )
#target_link_libraries(test_prop libdavix ${Boost_LIBRARIES} test_davix_functional_lib)

#add_executable(test_stat ${src_test_stat})
#target_link_libraries(test_stat libdavix test_davix_functional_lib)

#add_executable(test_stat_auto_check ${src_test_auto_stat_mkdir_check})
#target_link_libraries(test_stat_auto_check libdavix test_davix_functional_lib)

#add_executable(test_mkdir ${src_test_mkdir})
#target_link_libraries(test_mkdir libdavix test_davix_functional_lib)

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

#add_executable(test_listing ${src_test_listing} )
#target_link_libraries(test_listing libdavix test_davix_functional_lib)

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

#add_executable(test_valid_write_read_davfile ${src_test_valid_write_read_davfile})
#target_link_libraries(test_valid_write_read_davfile libdavix test_davix_functional_lib)

#add_executable(test_valid_write_read_vector_simple ${src_test_valid_write_read_vector_simple})
#target_link_libraries(test_valid_write_read_vector_simple libdavix test_davix_functional_lib)


#add_executable(tool_get_replicas ${src_tool_get_replicas})
#target_link_libraries(tool_get_replicas libdavix test_davix_functional_lib)

#add_executable(test_mv ${src_test_mv})
#target_link_libraries(test_mv libdavix test_davix_functional_lib)

add_executable(tester ${src_tester})
target_link_libraries(tester libdavix ${Boost_LIBRARIES} test_davix_functional_lib)

function(assert_not_empty var)
    if("${var}" STREQUAL "")
        message(FATAL_ERROR "variable must not be empty")
    endif()
endfunction(assert_not_empty)

function(matchparam param input)
    set(normalchars "a-zA-Z0-9/:.")
    string(REGEX MATCH "${param} +(([${normalchars}]-| |[${normalchars}])+)" tmp ${input})
    if(NOT "${tmp}" STREQUAL "")
        string(REPLACE " " "" stripped ${CMAKE_MATCH_1})
        set(match ${stripped} PARENT_SCOPE)
    else()
        set(match "" PARENT_SCOPE)
    endif()
endfunction(matchparam)

function(paramexists param input)
    string(REGEX MATCH "${param}" tmp ${input})
    set(match ${tmp} PARENT_SCOPE)
endfunction(paramexists)

function(runtest params)
    matchparam("--auth" ${params})
    set(auth ${match})
    assert_not_empty(${auth})

    matchparam("--command" ${params})
    set(command ${match})
    assert_not_empty(${command})

    matchparam("--uri" ${params})
    set(uri ${match})
    assert_not_empty(${uri})

    matchparam("--s3region" ${params})
    set(s3region ${match})

    paramexists("--s3alternate" ${params})
    set(s3alternate ${match})

    # build testname
    set(testname "${auth}")

    if(NOT "${s3region}" STREQUAL "")
        set(testname "${testname}|v4")
    endif()

    if(NOT "${s3alternate}" STREQUAL "")
        set(testname "${testname}|alt")
    endif()

    set(testname "${testname}|${command}|${uri}")

    set(arguments ${params})
    separate_arguments(arguments)
    add_test(${testname} tester ${arguments})
endfunction(runtest)

function(fulltest params)
    runtest("${params} --command makeCollection")
    runtest("${params} --command populate 5")
    runtest("${params} --command listing 5")
    runtest("${params} --command depopulate 3")
    runtest("${params} --command countfiles 2")
    runtest("${params} --command remove")
endfunction(fulltest)

function(s3tests params)
  runtest("${params} --command makeCollection")
  runtest("${params} --command populate 5")
  runtest("${params} --command depopulate 3")
  runtest("${params} --command countfiles 2")
  runtest("${params} --command depopulate 5")
endfunction(s3tests)

function(test_with_proxy uri)
    fulltest("--auth proxy --uri ${uri}")
endfunction(test_with_proxy)

function(test_s3 accesskey secretkey uri region use_alt)
    set(params "--auth aws --s3accesskey ${accesskey} --s3secretkey ${secretkey} --uri ${uri}")
    if(NOT "${region}" STREQUAL "")
        set(params "${params} --s3region ${region}")
    endif()

    if(use_alt MATCHES "^alt$")
        set(params "${params} --s3alternate")
    elseif(NOT use_alt MATCHES "^noalt$")
        message(FATAL_ERROR "Invalid option passed to test_s3 for use_alt: ${use_alt}")
    endif()

    s3tests(${params})
endfunction(test_s3)

function(test_azure azurekey uri)
    set(params "--auth azure --azurekey ${azurekey} --uri ${uri}")

    runtest("${params} --command populate 5")
    runtest("${params} --command statdir")
    runtest("${params} --command listing 5")
    runtest("${params} --command depopulate 5")
endfunction(test_azure)


include(${CMAKE_SOURCE_DIR}/credentials/creds.cmake)


### DEPRECATED, to remove soon

function(test_dav_endpoint_ronly name cred)

    add_test(test_stat_${name} test_stat ${name} ${cred})
    add_test(test_stat_session_${name} test_stat_session ${cred} ${name} ${name} ${name} ${name} ${name} ${name} ${name})
    add_test(test_opendir_${name} test_opendir ${name} ${cred})
  #  add_test(test_opendir_c_${name} test_opendir_c ${name} ${cred})
    add_test(test_opendirpp_${name} test_opendirpp ${name} ${cred})
  #  add_test(test_stat_c_${name} test_stat_c ${name} ${cred})
endfunction(test_dav_endpoint_ronly name cred)

function(test_dav_endpoint_rw name cred)

    add_test(test_stat_${name} test_stat ${name} ${cred})
    add_test(test_stat_session_${name} test_stat_session ${cred} ${name} ${name} ${name} ${name} ${name} ${name} ${name})
    add_test(test_opendir_${name} test_opendir ${name} ${cred})
    add_test(test_opendirpp_${name} test_opendirpp ${name} ${cred})
    add_test(test_listing_${name} test_listing ${name} ${cred})
    add_test(test_mkdir_${name} test_mkdir ${name} ${cred})
    add_test(test_stat_auto_check_${name} test_stat_auto_check ${name} ${cred})
endfunction(test_dav_endpoint_rw name cred)

function(listdir_partial name size cred)
    add_test(test_opendir_partial_${name}  test_opendir_partial ${name} ${size} ${cred})
endfunction(listdir_partial name size cred)


function(test_collection name cred)
    add_test(test_directory_${name} test_directory ${name} ${cred})
endfunction(test_collection name cred)

function(test_valid_read_generic url cred)
    add_test(test_valid_read_generic_${url} test_valid_read_generic_simple ${url} ${cred})
endfunction(test_valid_read_generic url cred)

function(test_valid_delete_all url cred)
    add_test(test_auto_rmdir_unlink_delete_${url} test_auto_rmdir_unlink_delete ${url} ${cred})
endfunction(test_valid_delete_all url cred)


function(test_valid_write_read_generic url cred)
    add_test(test_valid_write_read_generic_simple_${url} test_valid_write_read_generic_simple ${url} ${cred})
    add_test(test_rw_fd_${url} test_rw_fd ${url} ${cred})
    add_test(test_valid_write_read_vector_simple_${url} test_valid_write_read_vector_simple ${url} ${cred})
endfunction(test_valid_write_read_generic url cred)

function(test_replica_listing_existing url cred)
   add_test(tool_get_replicas_${url} tool_get_replicas ${url} ${cred})
endfunction(test_replica_listing_existing url cred)

function(test_rename url cred)
    add_test(test_mv_${url} test_mv ${url} ${cred})
endfunction(test_rename url cred)

# include(ctest_functional.cmake)

endif(FUNCTIONAL_TESTS)


