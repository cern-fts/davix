# ctest script file for automated functional tests for davix
#

message (" Setup tests parameters... ")

set(BASIC_LOGIN "test")
set(BASIC_PASSWD "tester")

set(http_dcache "http://sligo.desy.de:2880/pnfs/desy.de/data/dteam/davix-tests/" CACHE STRING "dCache test instance to use (read-only)")

set(https_dpm "https://dpmhead-trunk.cern.ch/dpm/cern.ch/home/dteam/davix-tests/" CACHE STRING "DPM test instance to use")
set(https_dpm_file "${https_dpm}/davix-tests/testread0011")
set(dav_dpm "davs://dpmhead-trunk.cern.ch/dpm/cern.ch/home/dteam/davix-tests/" CACHE STRING "DPM test instance to use throught WebDAV protocol")

set(https_dcache "https://prometheus.desy.de/VOs/dteam/" CACHE STRING "dCache test instance to use")

set(http_dynaFed_base "http://federation.desy.de/fed/dynafeds_demo/everywhere/")
set(http_dynaFed_file "${http_dynaFed_base}/file_1005.txt")

# set(metalink_url "http://download.documentfoundation.org/libreoffice/stable/4.3.4/deb/x86_64/LibreOffice_4.3.4_Linux_x86-64_deb.tar.gz")
# set(metalink3_url "http://download.documentfoundation.org/libreoffice/stable/4.3.4/deb/x86_64/LibreOffice_4.3.4_Linux_x86-64_deb_helppack_en-US.tar.gz.metalink")
# set(metalink_url_direct "http://download.documentfoundation.org/libreoffice/stable/4.3.4/deb/x86_64/LibreOffice_4.3.4_Linux_x86-64_deb_helppack_en-US.tar.gz.meta4")


# dCache read-only
test_dav_endpoint_ronly( "${http_dcache}" "proxy")

# dCache tests
test_valid_write_read_generic("${https_dcache}" "proxy")
test_rename("${https_dcache}" "proxy")
test_dav_endpoint_rw( "${https_dcache}" "proxy")
# test_valid_delete_all("${https_dcache}"  "proxy")
test_valid_read_generic("${https_dcache}" "proxy")
test_valid_write_read_generic("${https_dcache}" "proxy")


# DPM tests
test_valid_write_read_generic("${https_dpm}" "proxy")
test_rename("${https_dpm}" "proxy")
test_dav_endpoint_rw( "${https_dpm}" "proxy")
test_valid_delete_all("${https_dpm}"  "proxy")
test_valid_read_generic("${https_dpm}" "proxy")
test_valid_write_read_generic("${https_dpm}" "proxy")

# DPM Webdav
test_collection("${dav_dpm}" "proxy")

# Storm tests
## Disable storm test no valid gate !
#test_dav_endpoint_rw( "${http_storm_base}" "proxy")
#test_valid_delete_all("${http_storm_base}"  "proxy")
#test_valid_read_generic("${http_storm_base}" "proxy")
#test_valid_write_read_generic("${http_storm_base}" "proxy")

# localhost generic server, ex : "davserver -n -D /tmp"
test_dav_endpoint_rw("http://localhost:8008" "")

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
test_valid_read_generic("${https_dpm}" "proxy")
test_valid_delete_all("${https_dpm}"  "proxy")

# dynaFed
test_valid_read_generic("${http_dynaFed_base}" "proxy")

# test replicas listing
test_replica_listing_existing("${https_dpm_file}" "proxy")
test_replica_listing_existing("${http_dynaFed_file}" "proxy")

# test_replica_listing_existing("${metalink_url}" "proxy")
# test_replica_listing_existing("${metalink3_url}" "proxy")
# test_replica_listing_existing("${metalink_url_direct}" "proxy")
