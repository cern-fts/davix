## cmake build script for srm-ifce
project (davix)
cmake_minimum_required (VERSION 2.6)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")
option(shared_library "generate shared library" TRUE)
option(static_library  "generate static library" FALSE)


# build type
if(NOT DEFINED CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif(NOT DEFINED CMAKE_BUILD_TYPE)

# load module
include(DefineInstallationPaths REQUIRED)
include(ReleaseDebugAutoFlags REQUIRED)
include(CMakeGeneratePkgConfig REQUIRED)
include(MacroAddDoxygen REQUIRED)
include(PortabilityGNUCheck REQUIRED)
include(CMakeCXX11Support REQUIRED)
find_package(PkgConfig REQUIRED)

#enable testing
INCLUDE(CTest)


set(OUTPUT_NAME_DAVIX "davix")
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 28)
set(VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})


#compilation mode
SET(HTTP_LIB "NEON") # http backend can be NEON
SET(UNIT_TESTS FALSE
    CACHE STRING "enable or disable unit test compilation")
SET(NEON_EMBEDED TRUE
    CACHE STRING "enable or disable unit test compilation")

# libs checks
find_package(Threads)
add_subdirectory(deps)

pkg_check_modules(CRYPTO_PKG REQUIRED libcrypto)




## general defs
add_definitions( -D_GNU_SOURCE)
add_definitions( -DNEON_BACKEND)
add_definitions( -D__DAVIX_INSIDE__)
add_definitions( -DDAVIX_VERSION="${VERSION_STRING}" )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_FLAG_ENABLE}")

include_directories( ${HTTPLIB_PKG_INCLUDE_DIRS} ${GLIB2_PKG_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/deps)

install(FILES RELEASE-NOTES
		DESTINATION ${DOC_INSTALL_DIR}/) # install release notes

add_subdirectory (src)
add_subdirectory (doc)
add_subdirectory (test)
add_subdirectory (packaging)


#######################################################
# Configure an 'uninstall' target
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

