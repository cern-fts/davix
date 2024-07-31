#-------------------------------------------------------------------------------
# Make gtest / gmock available for all targets that need it
#-------------------------------------------------------------------------------
set(BUILD_SHARED_LIBS OFF)

add_executable(davix-slow-unit-tests
  ../drunk-server/ConnectionInitiator.cpp
  ../drunk-server/DrunkServer.cpp
  ../drunk-server/Interactors.cpp
  ../drunk-server/LineReader.cpp

  drunk-server.cpp
  standalone-request.cpp
)

target_include_directories(davix-slow-unit-tests PRIVATE
  ${PROJECT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(davix-slow-unit-tests
  davix_tool_lib
  libdavix
  GTest::GTest
  GTest::Main
  ${CMAKE_THREAD_LIBS_INIT}
  ${LIBSSL_PKG_LIBRARIES}
)

add_test(slow-unit-tests davix-slow-unit-tests)
