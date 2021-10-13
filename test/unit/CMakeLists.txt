#-------------------------------------------------------------------------------
# Make gtest / gmock available for all targets that need it
#-------------------------------------------------------------------------------
set(BUILD_SHARED_LIBS OFF)

set(GTEST "${CMAKE_SOURCE_DIR}/deps/googletest/googletest/")
set(GTEST_BINARY_DIR "${CMAKE_BINARY_DIR}/deps/googletest/googletest/")
add_subdirectory("${GTEST}" "${GTEST_BINARY_DIR}")

add_definitions( -DTEST_VALID_CERT="${TEST_CRED_PATH}" -DTEST_VALID_CERT_PASS="${TEST_CRED_PASS}")

add_executable(davix-unit-tests
  ../drunk-server/DrunkServer.cpp

  cache.cpp
  chrono.cpp
  config-parser.cpp
  content-provider.cpp
  context.cpp
  datetime.cpp
  digest-extractor.cpp
  gcloud.cpp
  metalink-replica.cpp
  neon.cpp
  parser.cpp
  response-buffer.cpp
  session-factory.cpp
  session.cpp
  status.cpp
  testcert.cpp
  typeconv.cpp
  utils.cpp
  xml-parser.cpp
)

target_include_directories(davix-unit-tests PRIVATE
  ${PROJECT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GTEST}/include
)

target_link_libraries(davix-unit-tests
  davix_tool_lib
  libdavix
  gtest_main
  gtest
  ${CMAKE_THREAD_LIBS_INIT}
  ${LIBSSL_PKG_LIBRARIES}
)

install(TARGETS davix-unit-tests
  DESTINATION ${BIN_INSTALL_DIR}/)

add_test(unit-tests davix-unit-tests)
