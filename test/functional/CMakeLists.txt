# main file for src

SET(FUNCTIONAL_TESTS TRUE
    CACHE STRING " enable or disable the functional tests")


if(FUNCTIONAL_TESTS)

FILE(GLOB src_test_req "test_request.cpp")
FILE(GLOB src_test_prop "test_propfind.cpp")
FILE(GLOB src_test_stat "test_stat.cpp")
FILE(GLOB src_test_mkdir "test_mkdir.cpp")
FILE(GLOB src_test_stat_session "test_stat_session.cpp")
FILE(GLOB src_test_opendir "test_opendir.cpp")
FILE(GLOB src_test_opendirpp "test_opendirpp.cpp")
FILE(GLOB src_test_stat_c "test_stat_c.c")

add_executable(test_request ${src_test_req} )
target_link_libraries(test_request davix)


add_executable(test_prop ${src_test_prop} )
target_link_libraries(test_prop davix)

add_executable(test_stat ${src_test_stat} )
target_link_libraries(test_stat davix)

add_executable(test_mkdir ${src_test_mkdir} )
target_link_libraries(test_mkdir davix)

add_executable(test_stat_session ${src_test_stat_session} )
target_link_libraries(test_stat_session davix)


add_executable(test_opendir ${src_test_opendir} )
target_link_libraries(test_opendir davix)

add_executable(test_opendirpp ${src_test_opendirpp} )
target_link_libraries(test_opendirpp davix)

add_executable(test_stat_c ${src_test_stat_c} )
target_link_libraries(test_stat_c davix)



function(test_dav_endpoint_ronly name cred)

    add_test(test_stat_${name} test_stat ${name} ${cred})
    add_test(test_stat_session_${name} test_stat_session ${cred} ${name} ${name} ${name} ${name} ${name} ${name} ${name})
    add_test(test_opendir_${name} test_opendir ${name} ${cred})
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
endfunction(test_dav_endpoint_rw name cred)


include(ctest_functional.cmake)

endif(FUNCTIONAL_TESTS)


