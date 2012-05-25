## ctest script file for automated functional tests for davix
#

message (" Setup tests parameters... ")


test_dav_endpoint( "http://sligo.desy.de:2880/" "")
test_dav_endpoint( "https://lxfsra04a04.cern.ch/dpm/" "${CMAKE_SOURCE_DIR}/test.p12")
