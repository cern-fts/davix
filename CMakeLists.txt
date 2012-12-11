## cmake build script for srm-ifce
project (davix)
cmake_minimum_required (VERSION 2.6)

# build type
set(CMAKE_BUILD_TYPE "RelWithDebInfo"
    CACHE STRING "build type")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")
# load module
include(DefineInstallationPaths REQUIRED)
include(MacroCopyFile REQUIRED)
include(ReleaseDebugAutoFlags REQUIRED)
include(CMakeGeneratePkgConfig REQUIRED)
include(MacroAddDoxygen REQUIRED)

find_package(PkgConfig REQUIRED)

#enable testing
INCLUDE(CTest)


set(OUTPUT_NAME_DAVIX "davix")
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 15)
set(VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})


#compilation mode
SET(HTTP_LIB "NEON") # http backend can be NEON
SET(UNIT_TESTS FALSE
    CACHE STRING "enable or disable unit test compilation")
SET(NEON_EMBEDED TRUE
    CACHE STRING "enable or disable unit test compilation")


# libs checks
pkg_check_modules(GTHREAD_PKG REQUIRED gthread-2.0)
pkg_check_modules(GLIB2_PKG REQUIRED glib-2.0)


if(NOT NEON_EMBEDED)
    pkg_check_modules(HTTPLIB_PKG REQUIRED neon)
else(NOT NEON_EMBEDED)
    add_subdirectory(deps)
endif(NOT NEON_EMBEDED)

pkg_check_modules(CRYPTO_PKG REQUIRED libcrypto)




## general defs
add_definitions( -D_GNU_SOURCE)
add_definitions( -DNEON_BACKEND)
add_definitions( -D__DAVIX_INSIDE__)


## patch against 0.25.0 problem of libneon
## valid in shared library mod only
if("${HTTPLIB_PKG_VERSION}" VERSION_LESS "0.27.0")
    message(" .... neon version < 0.27 detected : disable session re-use, risk of bug important")
    add_definitions( -D_DISABLE_SESSION_REUSE)
    add_definitions( -D_NEON_VERSION_0_25)
endif("${HTTPLIB_PKG_VERSION}" VERSION_LESS "0.27.0")



include_directories( ${HTTPLIB_PKG_INCLUDE_DIRS} ${GLIB2_PKG_INCLUDE_DIRS})

install(FILES RELEASE-NOTES
		DESTINATION ${DOC_INSTALL_DIR}/) # install release notes


add_subdirectory (src)
add_subdirectory (doc)
add_subdirectory (test)


#######################################################
# Configure an 'uninstall' target
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

