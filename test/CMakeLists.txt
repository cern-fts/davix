# main file for src

include_directories(${PROJECT_SOURCE_DIR}/src) 
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/gtest-1.6.0/include)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/gtest-1.6.0/src/)
link_directories(${PROJECT_BINARY_DIR}/src/)

add_subdirectory(gtest-1.6.0)
add_subdirectory(unit)
add_subdirectory(functional)




