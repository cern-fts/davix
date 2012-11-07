# main file for src

SET(FUNCTIONAL_TESTS FALSE
    CACHE STRING " enable or disable the functional tests")


if(FUNCTIONAL_TESTS)

set(src_davix_test_lib "davix_test_lib.c")

set( src_test_req "test_request.cpp")
set( src_test_prop "test_propfind.cpp")
set( src_test_stat "test_stat.cpp" )
set( src_test_mkdir "test_mkdir.cpp")
set( src_test_stat_session "test_stat_session.cpp")
set( src_test_rw "davix_rw_test.cpp")
set( src_test_opendir "test_opendir.cpp")
set( src_test_opendir_c "test_opendir_c.cpp")
set( src_test_opendir_partial "test_opendir_partial.cpp")
set( src_test_opendirpp "test_opendirpp.cpp")
set( src_test_stat_c "test_stat_c.c")
set( src_test_auto_stat_mkdir_check "test_auto_stat_mkdir_check.cpp")
set( src_test_valid_read_generic_simple "test_valid_read_generic_simple.cpp")
set( src_test_valid_write_read_generic_simple "test_valid_write_read_generic_simple.cpp")

add_executable(test_request ${src_test_req} )
target_link_libraries(test_request davix)


add_executable(test_prop ${src_test_prop} ${src_davix_test_lib} )
target_link_libraries(test_prop davix)

add_executable(test_stat ${src_test_stat} ${src_davix_test_lib})
target_link_libraries(test_stat davix)

add_executable(test_stat_auto_check ${src_test_auto_stat_mkdir_check} ${src_davix_test_lib})
target_link_libraries(test_stat_auto_check davix)

add_executable(test_mkdir ${src_test_mkdir} )
target_link_libraries(test_mkdir davix)

add_executable(test_stat_session ${src_test_stat_session} ${src_davix_test_lib})
target_link_libraries(test_stat_session davix)


add_executable(test_rw ${src_test_rw} ${src_davix_test_lib})
target_link_libraries(test_rw davix)

add_executable(test_opendir ${src_test_opendir} )
target_link_libraries(test_opendir davix)

add_executable(test_opendir_c ${src_test_opendir_c} ${src_davix_test_lib})
target_link_libraries(test_opendir_c davix)

add_executable(test_opendir_partial ${src_test_opendir_partial} ${src_davix_test_lib})
target_link_libraries(test_opendir_partial davix)

add_executable(test_opendirpp ${src_test_opendirpp} ${src_davix_test_lib})
target_link_libraries(test_opendirpp davix)

add_executable(test_stat_c ${src_test_stat_c} ${src_davix_test_lib})
target_link_libraries(test_stat_c davix)

add_executable(test_valid_read_generic_simple ${src_test_valid_read_generic_simple} ${src_davix_test_lib})
target_link_libraries(test_valid_read_generic_simple davix)

add_executable(test_valid_write_read_generic_simple ${src_test_valid_write_read_generic_simple} ${src_davix_test_lib})
target_link_libraries(test_valid_write_read_generic_simple davix)


function(test_dav_endpoint_ronly name cred)

    add_test(test_stat_${name} test_stat ${name} ${cred})
    add_test(test_stat_session_${name} test_stat_session ${cred} ${name} ${name} ${name} ${name} ${name} ${name} ${name})
    add_test(test_opendir_${name} test_opendir ${name} ${cred})
    add_test(test_opendir_c_${name} test_opendir_c ${name} ${cred})
    add_test(test_opendirpp_${name} test_opendirpp ${name} ${cred})
    add_test(test_stat_c_${name} test_stat_c ${name} ${cred})
endfunction(test_dav_endpoint_ronly name cred)

function(test_dav_endpoint_rw name cred)

    add_test(test_stat_${name} test_stat ${name} ${cred})
    add_test(test_stat_session_${name} test_stat_session ${cred} ${name} ${name} ${name} ${name} ${name} ${name} ${name})
    add_test(test_opendir_${name} test_opendir ${name} ${cred})
    add_test(test_opendirpp_${name} test_opendirpp ${name} ${cred})
    add_test(test_stat_c_${name} test_stat_c ${name} ${cred})
    add_test(test_mkdir_${name} test_mkdir ${name} ${cred})
    add_test(test_stat_auto_check_${name} test_stat_auto_check ${name} ${cred})
endfunction(test_dav_endpoint_rw name cred)

function(listdir_partial name size cred)
    add_test(test_opendir_partial_${name}  test_opendir_partial ${name} ${size} ${cred})
endfunction(listdir_partial name size cred)

function(test_valid_read_generic url cred)
    add_test(test_valid_read_generic_${url} test_valid_read_generic_simple ${url} ${cred})
endfunction(test_valid_read_generic url cred)

include(ctest_functional.cmake)

endif(FUNCTIONAL_TESTS)


