# main file for src

if(BENCH_TESTS)

LIST(APPEND src_davix_bench "davix_bench.cpp" "chunk_queue.cpp")

#include_directories(/usr/include/davix)

add_executable(davix-bench ${src_davix_bench})
target_link_libraries(davix-bench libdavix ${CMAKE_THREAD_LIBS_INIT})

function(test_read url opt input)
    add_test(test_bench_read_${url} davix-bench ${opt} ${url} ${input})
endfunction(test_read url opt)

function(test_vector_read url opt input)
    add_test(test_bench_vector_read_${url} davix-bench ${opt} ${url} ${input})
endfunction(test_vector_read url opt)

function(test_thread_read url opt input)
    add_test(test_bench_thread_read_${url} davix-bench ${opt} ${url} ${input})
endfunction(test_thread_read url opt)

function(test_write url opt input)
    add_test(test_bench_write_${url} davix-bench ${opt} ${url} ${input})
endfunction(test_write url opt)

include(ctest_bench.cmake)

endif(BENCH_TESTS)


