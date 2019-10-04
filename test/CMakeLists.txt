# main file for src

# force RPATH usage for easy test execution
set(CMAKE_SKIP_RPATH OFF
    CACHE BOOL "" FORCE)


include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src/)
link_directories(${PROJECT_BINARY_DIR}/src/)


set(TEST_CRED_PATH "${CMAKE_CURRENT_SOURCE_DIR}/certs/my_keycert.p12")
set(TEST_CRED_PASS "testpass")

add_subdirectory(unit)
add_subdirectory(slow-unit)
add_subdirectory(functional)
add_subdirectory(root-tests)
add_subdirectory(bench)



