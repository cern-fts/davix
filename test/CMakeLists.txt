# main file for src

# force RPATH usage for easy test execution
set(CMAKE_SKIP_RPATH OFF
    CACHE BOOL "" FORCE)

include_directories(${PROJECT_SOURCE_DIR}/src) 
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/gtest-1.6.0/include)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/gtest-1.6.0/src/)
link_directories(${PROJECT_BINARY_DIR}/src/)

set(TEST_CRED_PATH "${CMAKE_CURRENT_SOURCE_DIR}/certs/my_keycert.p12")
set(TEST_CRED_PASS "testpass")

add_subdirectory(gtest-1.6.0)
add_subdirectory(unit)
add_subdirectory(functional)




