# main file for src




FILE(GLOB _src_test_datetime "*.cpp")

set(src_test_datetime ${_src_test_datetime} PARENT_SCOPE)

#add_executable(test_datetime ${src_test_datetime})
#target_link_libraries(test_datetime cgreen m)


#add_test(test_datetime_ios8601 test_datetime test_datetime_iso8601)
