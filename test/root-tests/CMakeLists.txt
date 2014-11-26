


option(ROOT_TESTS "Root functional test execution " FALSE)

if(ROOT_TESTS)

## the root tests are compatibility tests with the ROOT analysis framework (http://root.cern.ch/)
## The root tests need to be compiled and executed :
## - the ROOT framework
## - a valid proxy credential for the dteam VO


find_package(ROOT REQUIRED)

add_definitions(-Wall)




add_definitions( ${INTERN_ROOT_CONFIG_CFLAGS} -lTreePlayer)
link_directories( ${ROOT_LIBRARY_DIR})
include_directories( ${ROOT_INCLUDE_DIR})
add_executable(test_event "h2fastnew_main.cpp")
target_link_libraries(test_event ${ROOT_LIBRARIES} TreePlayer)



function(test_root_event name url)

    add_test(test_root_event_${name} test_event ${url})
endfunction(test_root_event name url)


## official root website
test_root_event(root_website http://root.cern.ch/files/h1big.root)
## DPM test https
test_root_event(root_DPM_testbed https://lxfsra04a04.cern.ch/dpm/cern.ch/home/dteam/h1big.root)
## Dcache test http
test_root_event(root_dCache_testbed http://vm-dcache-deploy6.desy.de:2881/dteam/h1big.root)


endif(ROOT_TESTS)


