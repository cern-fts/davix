#
# This module detects if GRIDSITE is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# GRIDSITE_LIBRARIES       = full path to the GRIDSITE libraries
# GRIDSITE_SSL_LIBRARIES   = full path to the GRIDSITE ssl libraries
# GRIDSITE_INCLUDE_DIR     = include dir to be used when using the GRIDSITE library
# GRIDSITE_WSDL2H          = wsdl2h binary
# GRIDSITE_SOAPCPP2        = soapcpp2 binary
# GRIDSITE_FOUND           = set to true if GRIDSITE was found successfully
#
# GRIDSITE_LOCATION
#   setting this enables search for GRIDSITE libraries / headers in this location


# ------------------------------------------------------
# try pkg config search
#
# -----------------------------------------------------


find_package(PkgConfig)
pkg_check_modules(PC_GRIDSITE QUIET gridsite-openssl>=1.7.25)

IF(PC_GRIDSITE_FOUND)

SET(GRIDSITE_LIBRARIES ${PC_GRIDSITE_LIBRARIES})
SET(GRIDSITE_INCLUDE_DIR ${PC_GRIDSITE_INCLUDE_DIRS})
SET(GRIDSITE_DEFINITIONS "${PC_GRIDSITE_CFLAGS} ${PC_GRIDSITE_CFLAGS_OTHER}")

ELSE(PC_GRIDSITE_FOUND)

# -----------------------------------------------------
# GRIDSITE Libraries
# -----------------------------------------------------
find_library(GRIDSITE_LIBRARIES
    NAMES gridsite
    HINTS ${GRIDSITE_LOCATION}/lib ${GRIDSITE_LOCATION}/lib64 
          ${GRIDSITE_LOCATION}/lib32
    DOC "The main GRIDSITE library"
)

# -----------------------------------------------------
# GRIDSITE Include Directories
# -----------------------------------------------------
find_path(GRIDSITE_INCLUDE_DIR 
    NAMES gridsite.h
    HINTS ${GRIDSITE_LOCATION} ${GRIDSITE_LOCATION}/include ${GRIDSITE_LOCATION}/include/*
    DOC "The GRIDSITE include directory"
)

SET(GRIDSITE_DEFINITIONS "")

ENDIF(PC_GRIDSITE_FOUND)


# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set GRIDSITE_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GRIDSITE DEFAULT_MSG GRIDSITE_LIBRARIES 
    GRIDSITE_INCLUDE_DIR)
mark_as_advanced(GRIDSITE_INCLUDE_DIR GRIDSITE_LIBRARIES )
