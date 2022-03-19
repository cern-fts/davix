#-------------------------------------------------------------------------------
# Initialize
#-------------------------------------------------------------------------------
cmake_minimum_required (VERSION 3.0)
project (davix)

#-------------------------------------------------------------------------------
# Find the python executable to use during the build
#-------------------------------------------------------------------------------
find_package(Python COMPONENTS Interpreter REQUIRED)

#-------------------------------------------------------------------------------
# Regenerate include/davix/features.hpp and version.cmake at _build_ time
#-------------------------------------------------------------------------------
add_custom_target(GenerateVersionInfo ALL DEPENDS Version)
add_custom_command(
  OUTPUT Version
  COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/genversion.py --template include/davix/features.hpp.in --out include/davix/features.hpp
  COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/genversion.py --template version.cmake.in --out version.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

#-------------------------------------------------------------------------------
# Regenerate version.cmake at _configure_ time. Important since the above
# only regenerates it at compile time.
#-------------------------------------------------------------------------------
execute_process(
  COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/genversion.py --template version.cmake.in --out version.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

#-------------------------------------------------------------------------------
# Include version.cmake. Whenever the version changes, ie you commit or tag,
# the following happens:
# - Next time you run 'make', version.cmake is updated.
# - The next 'make' will detect version.cmake was changed, and cmake will
#   reconfigure itself.
#
# A bit hacky.
#-------------------------------------------------------------------------------
include(${CMAKE_CURRENT_SOURCE_DIR}/release.cmake REQUIRED)
include(${CMAKE_CURRENT_SOURCE_DIR}/version.cmake OPTIONAL)
message("Configuring cmake for davix version: ${VERSION_FULL}")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")
option(SHARED_LIBRARY "generate shared library" TRUE)
option(STATIC_LIBRARY  "generate static library" FALSE)


# build type
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to RelWithDebInfo")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif(NOT CMAKE_BUILD_TYPE)

# load module
include(DefineInstallationPaths REQUIRED)
include(ReleaseDebugAutoFlags REQUIRED)
include(CMakeGeneratePkgConfig REQUIRED)
include(MacroAddDoxygen REQUIRED)
include(CMakeCXX11Support REQUIRED)
include(PortabilityGNUCheck REQUIRED)



set(OUTPUT_NAME_DAVIX "davix")

# components
option(ENABLE_TOOLS "enable or disable tools " TRUE)
option(ENABLE_HTML_DOCS "enable or disable generation of HTML documentation" FALSE)
option(EMBEDDED_LIBCURL "Use embedded libcurl" TRUE)
option(LIBCURL_BACKEND_BY_DEFAULT "Use libcurl by default" FALSE)

# features
option(ENABLE_IPV6 "enable or disable IPv6 support " TRUE)
option(ENABLE_TCP_NODELAY "enable or disable tcp_nodelay" TRUE)
option(ENABLE_THIRD_PARTY_COPY "enable or disable third party copy support" FALSE)

# tests
option(BENCH_TESTS "enable or disable the bench tests" FALSE)
option(DAVIX_TESTS "Flag to disable the building of all tests" TRUE)

# libs checks
find_package(Threads)

# openssl
find_package(OpenSSL)
SET(LIBSSL_PKG_LIBRARIES "${OPENSSL_LIBRARIES}")
SET(LIBSSL_PKG_INCLUDE_DIRS "${OPENSSL_INCLUDE_DIR}")

find_package(LibXml2)
set(LIBXML2_INCLUDE_DIRS ${LIBXML2_INCLUDE_DIR})

#const
set(HAVE_OPENSSL 1)
set(NE_HAVE_SSL 1)

add_subdirectory(deps)


#libuuid
if(NOT APPLE)
  find_package(uuid)
endif()

## general defs
add_definitions( -D_GNU_SOURCE)              # GNU source, import LFS, etc...
add_definitions( -D__DAVIX_INSIDE__)         # protection flag
add_definitions( -D__STDC_LIMIT_MACROS )     # C99 limit macros


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_FLAG_ENABLE}")

#-------------------------------------------------------------------------------
# Are we using system, or embedded libcurl?
#-------------------------------------------------------------------------------
if(EMBEDDED_LIBCURL)
  include(buildCurl REQUIRED)
  buildCurl()
  set(libcurl libcurl)
else()
  find_package(CURL REQUIRED)
  set(libcurl CURL::libcurl)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/libneon/src/ ${UUID_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/include/davix)

add_subdirectory (src)
add_subdirectory (doc)

if(EXISTS "${CMAKE_SOURCE_DIR}/dist/CMakeLists.txt")
  add_subdirectory (dist)
endif()

if(DAVIX_TESTS)
  add_subdirectory (test)
endif()

#############################################################################
## headers
configure_file("${CMAKE_SOURCE_DIR}/include/davix/features.hpp.in"
               "${CMAKE_SOURCE_DIR}/include/davix/features.hpp"
               @ONLY)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/davix/
        DESTINATION ${INCLUDE_INSTALL_DIR}/davix
        PATTERN "*.in" EXCLUDE)

##############################################################################
## Doc file
install(FILES RELEASE-NOTES.md LICENSE
                DESTINATION ${DOC_INSTALL_DIR}/) # install release notes


#######################################################
# Configure an 'uninstall' target
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

# ADD_CUSTOM_TARGET(uninstall
#   "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
