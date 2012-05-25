## ctest script file for automated functional tests for davix
#

message (" Setup tests parameters... ")


test_dav_endpoint_ronly( "http://sligo.desy.de:2880/pnfs/desy.de/data/dteam/generated" "")
test_dav_endpoint_rw( "https://lxfsra04a04.cern.ch/dpm/cern.ch/home/dteam/generated" "${CMAKE_SOURCE_DIR}/test.p12")
listdir_partial("https://lxfsra04a04.cern.ch/dpm/cern.ch/home/dteam/dir10k" 2000 "${CMAKE_SOURCE_DIR}/test.p12")

# localhost generic server, ex : "davserver -n -D /tmp"
test_dav_endpoint_rw("http://localhost:8008" " ")
