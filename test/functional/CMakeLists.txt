# main file for src

SET(FUNCTIONAL_TESTS FALSE
    CACHE STRING " enable or disable the functional tests")


if(FUNCTIONAL_TESTS)

FILE(GLOB src_test_req "test_request.cpp")
FILE(GLOB src_test_prop "test_propfind.cpp")
FILE(GLOB src_test_stat "test_stat.cpp")
FILE(GLOB src_test_stat_c "test_stat_c.c")

add_executable(test_request ${src_test_req} )
target_link_libraries(test_request davix)


add_executable(test_prop ${src_test_prop} )
target_link_libraries(test_prop davix)

add_executable(test_stat ${src_test_stat} )
target_link_libraries(test_stat davix)

add_executable(test_stat_c ${src_test_stat_c} )
target_link_libraries(test_stat_c davix)


endif(FUNCTIONAL_TESTS)


