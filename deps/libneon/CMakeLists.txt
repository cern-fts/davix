project(libneon-tuned)
cmake_minimum_required (VERSION 2.6)
# main file for src

SET(SSL_BACKEND "OPENSSL"
    CACHE STRING "set the default SSL backend openssl or gnutls ( default : openssl ) ")

OPTION(ENABLE_IPV6 "enable or disable IPv6 support " "1")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

# load module
include(ReleaseDebugAutoFlags REQUIRED)
include(PortabilityGNUCheck REQUIRED)


set(OUTPUT_NAME_DAVIX "neon")
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 29)
set(VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

find_package(SOCKET)

find_package(LibXml2)
if(LIBXML2_INCLUDE_DIR)
set(HAVE_LIBXML 1)
endif(LIBXML2_INCLUDE_DIR)

if(${SSL_BACKEND} STREQUAL "OPENSSL")
pkg_search_module(LIBSSL_PKG REQUIRED libssl)
set(HAVE_OPENSSL 1)
set(NE_HAVE_SSL 1)
endif(${SSL_BACKEND} STREQUAL "OPENSSL")

set(HAVE_IPV6 ${ENABLE_IPV6})



configure_file(config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../
                    ${LIBXML2_INCLUDE_DIR} )

add_definitions( -fPIC -DNEON_VERSION="${VERSION_STRING}" -DNE_VERSION_MAJOR=${VERSION_MAJOR}  -DNE_VERSION_MINOR=${VERSION_MINOR})



add_subdirectory(src)
