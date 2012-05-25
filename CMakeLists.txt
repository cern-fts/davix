## cmake build script for srm-ifce
project (libdavix)
cmake_minimum_required (VERSION 2.6)

# build type
set(CMAKE_BUILD_TYPE RelWithDebInfo
	CACHE STRING "type of build")

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


set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})


#compilation mode
SET(HTTP_LIB "NEON") # http bacend can be NEON or CURL


# libs checks
pkg_check_modules(GLIBMM_PKG REQUIRED glibmm-2.4)

if ( ${HTTP_LIB} STREQUAL "CURL")
pkg_check_modules(HTTPLIB_PKG REQUIRED libcurl)
endif ( ${HTTP_LIB} STREQUAL "CURL")

if ( ${HTTP_LIB} STREQUAL "NEON" )
pkg_check_modules(HTTPLIB_PKG REQUIRED neon)
endif ( ${HTTP_LIB} STREQUAL "NEON" )

pkg_check_modules(LIBXMLPP_PKG REQUIRED libxml++-2.6)


if ( ${HTTP_LIB} STREQUAL "NEON" )
    add_definitions( -DNEON_BACKEND)
else ( ${HTTP_LIB} STREQUAL "NEON" )
    add_definitions( -DCURL_BACKEND)
endif ( ${HTTP_LIB} STREQUAL "NEON" )


include_directories(${GLIBMM_PKG_INCLUDE_DIRS} ${HTTPLIB_PKG_INCLUDE_DIRS})
include_directories(${LIBXMLPP_PKG_INCLUDE_DIRS})



add_subdirectory (src)
add_subdirectory (test)
