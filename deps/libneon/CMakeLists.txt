project(libneon-tuned)
cmake_minimum_required (VERSION 3.0)
# main file for src

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

# load module
include(ReleaseDebugAutoFlags REQUIRED)
include(PortabilityGNUCheck REQUIRED)
set(OUTPUT_NAME_DAVIX "neon")
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 29)
set(VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

set(HAVE_LIBXML 1)
set(HAVE_IPV6 ${ENABLE_IPV6})
set(TCP_NODELAY ${ENABLE_TCP_NODELAY})

configure_file(config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

add_library(LibNeonObjects OBJECT
  src/ne_207.c
  src/ne_acl3744.c
  src/ne_alloc.c
  src/ne_auth.c
  src/ne_basic.c
  src/ne_compress.c
  src/ne_dates.c
  src/ne_i18n.c
  src/ne_locks.c
  src/ne_md5.c
  src/ne_ntlm.c
  src/ne_oldacl.c
  src/ne_pkcs11.c
  src/ne_props.c
  src/ne_redirect.c
  src/ne_request.c
  src/ne_session.c
  src/ne_socket.c
  src/ne_socks.c
  src/ne_sspi.c
  src/ne_string.c
  src/ne_uri.c
  src/ne_utils.c
  src/ne_xml.c
  src/ne_xmlreq.c
  src/ne_openssl.c
)

target_compile_options(LibNeonObjects PRIVATE
  -std=gnu99 -fPIC -DNEON_VERSION="${VERSION_STRING}"
  -DNE_VERSION_MAJOR=${VERSION_MAJOR} -DNE_VERSION_MINOR=${VERSION_MINOR}
  -Wno-unused-parameter -fvisibility=hidden
)

target_include_directories(LibNeonObjects
  PRIVATE ${LIBXML2_INCLUDE_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR}
          ${CMAKE_CURRENT_BINARY_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR}/../
          ${OPENSSL_INCLUDE_DIR}
)
