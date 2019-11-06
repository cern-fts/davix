macro(buildCurl)
  include(ExternalProject)

  ExternalProject_Add(BuildCurlBundled
      SOURCE_DIR "${CMAKE_SOURCE_DIR}/deps/curl"
      PREFIX "${CMAKE_BINARY_DIR}/curl"
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND bash -c "autoreconf -i && ./configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/curl"
      BUILD_COMMAND ${MAKE}
  )

  add_library(libcurl STATIC IMPORTED)
  set_property(TARGET libcurl PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/curl/lib/libcurl.a)
  add_dependencies(libcurl BuildCurlBundled)
  set(CURL_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/curl/include)
endmacro()
