## ctest script file for automated bench tests for davix
#

message (" Setup tests parameters... ")

set(INPUT_FILE "-iinputfile.txt")
set(READ_OPT "-r")
set(READ_VECTOR_OPT "-v50")
set(READ_THREAD_OPT "-t4")
set(WRITE_OPT "-w")

set(http_desy_base "https://vm-dcache-deploy6.desy.de:2880/dteam/davix-tests" CACHE STRING "dCache test instance to use")
set(http_lcgdm_base "https://lxfsra04a04.cern.ch/dpm/cern.ch/home/dteam" CACHE STRING "DPM test instance to use" )
set(http_lcgdm_base_write "https://lxfsra04a04.cern.ch/dpm/cern.ch/home/dteam/davix_bench_writetest" CACHE STRING "DPM test instance to use" )

# DPM tests
test_read("${http_lcgdm_base}" "${READ_OPT}" "${INPUT_FILE}")
test_vector_read("${http_lcgdm_base}" "${READ_VECTOR_OPT}" "${INPUT_FILE}")
test_thread_read("${http_lcgdm_base}" "${READ_THREAD_OPT}" "${INPUT_FILE}")
test_write("${http_lcgdm_base_write}" "${WRITE_OPT}" "${INPUT_FILE}")

test_read("${http_desy_base}" "${READ_OPT}" "${INPUT_FILE}")
test_vector_read("${http_desy_base}" "${READ_VECTOR_OPT}" "${INPUT_FILE}")
test_thread_read("${http_desy_base}" "${READ_THREAD_OPT}" "${INPUT_FILE}")
test_write("${http_desy_base}" "${WRITE_OPT}" "${INPUT_FILE}")
