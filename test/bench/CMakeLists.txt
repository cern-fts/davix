project (davixtest)
cmake_minimum_required (VERSION 2.6)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package(Threads)

LIST(APPEND src_davix_bench "davix_bench.cpp" "chunk_queue.cpp")

include_directories(/usr/include/davix)
add_executable(davix-bench ${src_davix_bench})
target_link_libraries(davix-bench davix ${CMAKE_THREAD_LIBS_INIT})

