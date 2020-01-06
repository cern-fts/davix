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

find_package(SOCKET)

find_package(LibXml2)
if(LIBXML2_INCLUDE_DIR)
set(HAVE_LIBXML 1)
endif(LIBXML2_INCLUDE_DIR)

set(HAVE_IPV6 ${ENABLE_IPV6})
set(TCP_NODELAY ${ENABLE_TCP_NODELAY})

configure_file(config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../
                    ${LIBXML2_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})

add_definitions(  -std=gnu99 -fPIC -DNEON_VERSION="${VERSION_STRING}" -DNE_VERSION_MAJOR=${VERSION_MAJOR}  -DNE_VERSION_MINOR=${VERSION_MINOR} -Wno-unused-parameter)
add_definitions( -fvisibility=hidden)



add_subdirectory(src)
