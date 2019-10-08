#-------------------------------------------------------------------------------
# Make gtest / gmock available for all targets that need it
#-------------------------------------------------------------------------------
set(BUILD_SHARED_LIBS OFF)

set(GTEST "${CMAKE_SOURCE_DIR}/deps/googletest/googletest/")
set(GTEST_BINARY_DIR "${CMAKE_BINARY_DIR}/deps/googletest/googletest/")

include_directories(
  ${PROJECT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GTEST}/include
)


add_executable(davix-slow-unit-tests
  ../drunk-server/ConnectionInitiator.cpp
  ../drunk-server/DrunkServer.cpp
  ../drunk-server/Interactors.cpp
  ../drunk-server/LineReader.cpp

  drunk-server.cpp
  standalone-neon-request.cpp

  ${SRC_HMACSHA1} ${src_files} ${src_alibxx}
)

target_link_libraries(davix-slow-unit-tests
  alibxx_core
  davix_tool_lib
  libdavix
  gtest_main
  ${CMAKE_THREAD_LIBS_INIT}
  ${LIBSSL_PKG_LIBRARIES}
)
