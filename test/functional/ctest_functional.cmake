## ctest script file for automated functional tests for davix
#

message (" Setup tests parameters... ")

set(BASIC_LOGIN "test")
set(BASIC_PASSWD "tester")

set(http_desy_base "https://lcg-lrz-dc66.grid.lrz.de/pnfs/lrz-muenchen.de/data/dteam/davix-tests" CACHE STRING "dCache test instance to use")
set(http_desy_file "${http_desy_base}/fbxtest.txt" CACHE STRING "dCache file to sue for read only tests")
set(http_lcgdm_base "https://lxfsra04a04.cern.ch/dpm/cern.ch/home/dteam" CACHE STRING "DPM test instance to use" )


test_dav_endpoint_ronly( ${http_desy_base} "proxy")

test_dav_endpoint_rw( "${http_lcgdm_base}" "proxy")

test_valid_delete_all("${http_lcgdm_base}"  "proxy")
test_valid_read_generic("${http_lcgdm_base}" "proxy")

# localhost generic server, ex : "davserver -n -D /tmp"
test_dav_endpoint_rw("http://localhost:8008" "")

# localhost generic server with basic auth on port 8009,
# ex : "davserver -u test -p tester -P 8009 -D /tmp"
test_dav_endpoint_rw("http://localhost:8009" "${BASIC_LOGIN}:${BASIC_PASSWD}")
test_valid_delete_all("http://localhost:8009"  "${BASIC_LOGIN}:${BASIC_PASSWD}")


test_valid_delete_all("http://localhost:8008"  "${CMAKE_SOURCE_DIR}/test.p12")

# crap
#listdir_partial("${http_lcgdm_base}" 2000 "proxy")

## generic http query test
test_valid_read_generic("http://google.com" "")
test_valid_read_generic("http://wikipedia.org" "")
test_valid_read_generic("https://wikipedia.org" "")
test_valid_read_generic("http://www.w3.org/" "")
test_valid_read_generic("http://cern.ch" "")

# testwith common SE
test_valid_read_generic("${http_desy_base}" "proxy")
test_valid_read_generic("${http_desy_file}" "proxy")
test_valid_delete_all("${http_desy_base}"  "proxy")



test_valid_write_read_generic("${http_lcgdm_base}" "proxy")
test_valid_write_read_generic("${http_desy_base}" "proxy")
