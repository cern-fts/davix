# main file for src

SET(UNIT_TESTS FALSE
    CACHE STRING " enable or disable the unit tests")

if(UNIT_TESTS)

include_directories(${PROJECT_SOURCE_DIR}/src ".")


file(GLOB src_neon neon/*.cpp)
file(GLOB src_datetime datetime/*.cpp)
file(GLOB src_credential credential/*.cpp)
file(GLOB src_context context/*.cpp)
file(GLOB src_session session/*.cpp)
file(GLOB src_parser xmlparser/*.cpp)


add_executable(g_test_unit_davix_exe ${src_context} ${src_session} ${src_credential} ${src_datetime} ${src_neon} ${src_parser})
target_link_libraries(g_test_unit_davix_exe davix gtest gtest_main)


add_test(g_test_unit_davix g_test_unit_davix_exe )

endif(UNIT_TESTS)
