# main file for src

include_directories(${PROJECT_SOURCE_DIR}/src ".")




add_subdirectory(datetime)
add_subdirectory(session)


FILE(GLOB src_cgreen "*.cpp")

add_executable(test_unit_davix ${src_cgreen} ${src_test_datetime} ${src_test_session})
target_link_libraries(test_unit_davix davix cgreen)


add_test(test_davix_all test_unit_davix)
