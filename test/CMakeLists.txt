# main file for src

# force RPATH usage for easy test execution
set(CMAKE_SKIP_RPATH OFF
    CACHE BOOL "" FORCE)

include_directories(${PROJECT_SOURCE_DIR}/src) 
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/gtest-1.6.0/include)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/gtest-1.6.0/src/)
link_directories(${PROJECT_BINARY_DIR}/src/)

add_subdirectory(gtest-1.6.0)
add_subdirectory(unit)
add_subdirectory(functional)




