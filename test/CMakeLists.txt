# main file for src

# force RPATH usage for easy test execution
set(CMAKE_SKIP_RPATH OFF
    CACHE BOOL "" FORCE)


include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src/)
link_directories(${PROJECT_BINARY_DIR}/src/)


set(TEST_CRED_PATH "${CMAKE_CURRENT_SOURCE_DIR}/certs/my_keycert.p12")
set(TEST_CRED_PASS "testpass")

IF(NOT GTEST_EXTERNAL)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/gtest-1.6.0/include)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/gtest-1.6.0/src/)

add_subdirectory(gtest-1.6.0)
ELSE(NOT GTEST_EXTERNAL)
    SET(GTEST_INCLUDE_DIRS "/usr/include/gtest")
ENDIF(NOT GTEST_EXTERNAL)

add_subdirectory(unit)
add_subdirectory(functional)




