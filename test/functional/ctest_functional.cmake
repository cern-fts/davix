## ctest script file for automated functional tests for davix
#

message (" Setup tests parameters... ")

set(BASIC_LOGIN "test")
set(BASIC_PASSWD "tester")

set(http_desy_base "http://sligo.desy.de:2880/pnfs/desy.de/data/dteam")
set(http_desy_file "${http_desy_base}/fbxtest.txt")

set(http_lcgdm_base "https://lxfsra10a01.cern.ch/dpm/cern.ch/home/dteam")


test_dav_endpoint_ronly( ${http_desy_base} "")

test_dav_endpoint_rw( "${http_lcgdm_base}" "${CMAKE_SOURCE_DIR}/test.p12")
listdir_partial("${http_lcgdm_base}" 2000 "${CMAKE_SOURCE_DIR}/test.p12")
test_valid_delete_all("${http_lcgdm_base}"  "${CMAKE_SOURCE_DIR}/test.p12")
test_valid_read_generic("${http_lcgdm_base}" "${CMAKE_SOURCE_DIR}/test.p12")

# localhost generic server, ex : "davserver -n -D /tmp"
test_dav_endpoint_rw("http://localhost:8008" " ")

# localhost generic server with basic auth on port 8009,
# ex : "davserver -u test -p tester -P 8009 -D /tmp"
test_dav_endpoint_rw("http://localhost:8009" "${BASIC_LOGIN}:${BASIC_PASSWD}")
test_valid_delete_all("http://localhost:8009"  "${BASIC_LOGIN}:${BASIC_PASSWD}")


test_valid_delete_all("http://localhost:8008"  "${CMAKE_SOURCE_DIR}/test.p12")


## generic http query test
test_valid_read_generic("http://google.com" "")
test_valid_read_generic("http://wikipedia.org" "")
test_valid_read_generic("https://wikipedia.org" "")
test_valid_read_generic("http://www.w3.org/" "")
test_valid_read_generic("http://cern.ch" "")

# testwith common SE
test_valid_read_generic("${http_desy_base}" "")
test_valid_read_generic("${http_desy_file}" "")
test_valid_delete_all("${http_desy_base}"  "")



test_valid_write_read_generic("${http_lcgdm_base}" "${CMAKE_SOURCE_DIR}/test.p12")
test_valid_write_read_generic("${http_desy_base}" "${CMAKE_SOURCE_DIR}/test.p12")
