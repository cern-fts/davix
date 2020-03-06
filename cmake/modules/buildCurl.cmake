macro(buildCurl)
  include(ExternalProject)

  set(SECURE_TRANSPORT_FLAGS "")
  if(APPLE)
    set(SECURE_TRANSPORT_FLAGS "--with-secure-transport")
  endif()

  string(JOIN " "
    DISABLE_CURL_FEATURES
    "--disable-ftp"
    "--disable-ldap"
    "--disable-ldaps"
    "--disable-rtsp"
    "--disable-dict"
    "--disable-telnet"
    "--disable-tftp"
    "--disable-pop3"
    "--disable-imap"
    "--disable-smb"
    "--disable-smtp"
    "--disable-gopher"
    "--disable-sspi"
    "--disable-ntlm-wb"
    "--disable-netrc"

    "--without-winssl"
    "--without-schannel"
    "--without-darwinssl"
    "--without-amissl"
    "--without-gnutls"
    "--without-mbedtls"
    "--without-wolfssl"
    "--without-mesalink"
    "--without-nss"
    "--without-libpsl"
    "--without-libmetalink"
    "--without-librtmp"
    "--without-winidn"

    "--without-libidn2"
  )

  ExternalProject_Add(BuildCurlBundled
      SOURCE_DIR "${CMAKE_SOURCE_DIR}/deps/curl"
      PREFIX "${CMAKE_BINARY_DIR}/curl"
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND bash -c "autoreconf -i && ./configure  --prefix=${CMAKE_CURRENT_BINARY_DIR}/curl ${DISABLE_CURL_FEATURES} ${SECURE_TRANSPORT_FLAGS} && ${CMAKE_SOURCE_DIR}/patch-curl-clock-gettime.sh"
      BUILD_COMMAND ${MAKE}
  )

  add_library(libcurl STATIC IMPORTED)
  set_property(TARGET libcurl PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/curl/lib/libcurl.a)
  add_dependencies(libcurl BuildCurlBundled)

  # Replace with the following when possible:
  # target_include_directories(libcurl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/deps/curl/include)
  set_property(TARGET libcurl APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/deps/curl/include)
endmacro()
