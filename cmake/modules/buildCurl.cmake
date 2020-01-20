macro(buildCurl)
  include(ExternalProject)

  set(SECURE_TRANSPORT_FLAGS "")
  if(APPLE)
    set(SECURE_TRANSPORT_FLAGS "--with-secure-transport")
  endif()

  ExternalProject_Add(BuildCurlBundled
      SOURCE_DIR "${CMAKE_SOURCE_DIR}/deps/curl"
      PREFIX "${CMAKE_BINARY_DIR}/curl"
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND bash -c "autoreconf -i && ./configure  --prefix=${CMAKE_CURRENT_BINARY_DIR}/curl --disable-ldap --disable-ldaps ${SECURE_TRANSPORT_FLAGS} && ${CMAKE_SOURCE_DIR}/patch-curl-clock-gettime.sh"
      BUILD_COMMAND ${MAKE}
  )

  add_library(libcurl STATIC IMPORTED)
  set_property(TARGET libcurl PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/curl/lib/libcurl.a)
  add_dependencies(libcurl BuildCurlBundled)
  target_include_directories(libcurl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/deps/curl/include)
endmacro()
