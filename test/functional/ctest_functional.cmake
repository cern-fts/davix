## ctest script file for automated functional tests for davix
#

message (" Setup tests parameters... ")

set(BASIC_LOGIN "test")
set(BASIC_PASSWD "tester")

set(http_desy_base "https://vm-dcache-deploy6.desy.de:2880/dteam/davix-tests" CACHE STRING "dCache test instance to use")
set(http_lcgdm_base "https://lxfsra04a04.cern.ch/dpm/cern.ch/home/dteam" CACHE STRING "DPM test instance to use" )
#set(http_lcgdm_base "https://lxfsra10a01.cern.ch/dpm/cern.ch/home/dteam" CACHE STRING "DPM test instance to use" )
set(http_storm_base "https://gridhttps-storm-atlas.cr.cnaf.infn.it:8443/webdav/dteam/davix-tests/" CACHE STRING "Storm test instance to use")
set(http_dynaFed_base "http://dav-federation.cern.ch/myfed/dteam/")
set(http_lcgdm_file "${http_lcgdm_base}/davix-tests/testread0011")
set(http_dynaFed_file "${http_dynaFed_base}/davix-tests/testread0011")

set(metalink_url "http://download.documentfoundation.org/libreoffice/stable/4.3.0/deb/x86_64/LibreOffice_4.3.0_Linux_x86-64_deb.tar.gz")
set(metalink3_url "http://download.documentfoundation.org/libreoffice/stable/4.3.0/deb/x86_64/LibreOffice_4.3.0_Linux_x86-64_deb_helppack_en-US.tar.gz.metalink")
set(metalink_url_direct "http://download.documentfoundation.org/libreoffice/stable/4.3.0/deb/x86_64/LibreOffice_4.3.0_Linux_x86-64_deb_langpack_fr.tar.gz.meta4")

test_dav_endpoint_ronly( ${http_desy_base} "proxy")
test_valid_write_read_generic("${http_desy_base}" "proxy")

# DPM tests
test_dav_endpoint_rw( "${http_lcgdm_base}" "proxy")
test_valid_delete_all("${http_lcgdm_base}"  "proxy")
test_valid_read_generic("${http_lcgdm_base}" "proxy")
test_valid_write_read_generic("${http_lcgdm_base}" "proxy")

# Storm tests
test_dav_endpoint_rw( "${http_storm_base}" "proxy")
test_valid_delete_all("${http_storm_base}"  "proxy")
test_valid_read_generic("${http_storm_base}" "proxy")
test_valid_write_read_generic("${http_storm_base}" "proxy")

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
test_valid_read_generic("${http_dynaFed_base}" "proxy")
test_valid_delete_all("${http_desy_base}"  "proxy")


# test replicas listing
test_replica_listing_existing("${http_lcgdm_file}" "proxy")
test_replica_listing_existing("${http_dynaFed_file}" "proxy")
test_replica_listing_existing("${metalink_url}" "proxy")
test_replica_listing_existing("${metalink3_url}" "proxy")
test_replica_listing_existing("${metalink_url_direct}" "proxy")
