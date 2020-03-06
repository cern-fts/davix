macro(buildCurl)
  include(ExternalProject)

  set(SECURE_TRANSPORT_FLAGS "")
  if(APPLE)
    set(SECURE_TRANSPORT_FLAGS "-DCMAKE_USE_SECTRANSP=ON -DCMAKE_USE_OPENSSL=OFF -DCURL_CA_PATH=none")
  endif()

  ExternalProject_Add(BuildCurlBundled
      SOURCE_DIR "${CMAKE_SOURCE_DIR}/deps/curl"
      BINARY_DIR "${CMAKE_BINARY_DIR}/deps/curl"
      PREFIX "${CMAKE_BINARY_DIR}/deps/curl"
      CONFIGURE_COMMAND bash -c "${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=/usr/ -DCMAKE_INSTALL_LIBDIR=lib -DHTTP_ONLY=ON -DBUILD_CURL_EXE=OFF -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_USE_LIBSSH2=OFF ${SECURE_TRANSPORT_FLAGS} ${CMAKE_SOURCE_DIR}/deps/curl && ${CMAKE_SOURCE_DIR}/patch-curl-clock-gettime.sh"
      BUILD_COMMAND make
      INSTALL_COMMAND make DESTDIR=${CMAKE_BINARY_DIR}/deps/curl-install install
  )

  add_library(libcurl STATIC IMPORTED)
  set_property(TARGET libcurl PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/deps/curl-install/usr/lib/libcurl.a)
  add_dependencies(libcurl BuildCurlBundled)

  # Replace with the following when possible:
  # target_include_directories(libcurl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/deps/curl/include)
  set_property(TARGET libcurl APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/deps/curl/include)

endmacro()
