#
# This module detects if CGSI_GSOAP is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# CGSI_GSOAP_LIBRARIES   = full path to the CGSI_GSOAP libraries
# CGSI_GSOAP_INCLUDE_DIR = include dir to be used when using the CGSI_GSOAP library
# CGSI_GSOAP_FOUND       = set to true if CGSI_GSOAP was found successfully
#
# CGSI_GSOAP_LOCATION
#   setting this enables search for CGSI_GSOAP libraries / headers in this location


# -----------------------------------------------------
# CGSI_GSOAP Libraries
# -----------------------------------------------------
find_library(CGSI_GSOAP_LIBRARIES
    NAMES cgsi_plugin cgsi_plugin_gsoap_2.7
    HINTS ${CGSI_GSOAP_LOCATION}/lib ${CGSI_GSOAP_LOCATION}/lib64 ${CGSI_GSOAP_LOCATION}/lib32 ${STAGE_DIR}/lib ${STAGE_DIR}/lib64
    DOC "The main CGSI_GSOAP library"
)

# -----------------------------------------------------
# CGSI_GSOAP Include Directories
# -----------------------------------------------------
find_path(CGSI_GSOAP_INCLUDE_DIR 
    NAMES cgsi_plugin.h
    HINTS ${CGSI_GSOAP_LOCATION} ${CGSI_GSOAP_LOCATION}/include ${CGSI_GSOAP_LOCATION}/include/* ${STAGE_DIR}/include ${STAGE_DIR}/include
    DOC "The CGSI_GSOAP include directory"
)
if(CGSI_GSOAP_INCLUDE_DIR)
    message(STATUS "CGSI_GSOAP includes found in ${CGSI_GSOAP_INCLUDE_DIR}")
endif()



# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set CGSI_GSOAP_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CGSI_GSOAP DEFAULT_MSG CGSI_GSOAP_LIBRARIES CGSI_GSOAP_INCLUDE_DIR)
mark_as_advanced(CGSI_GSOAP_INCLUDE_DIR CGSI_GSOAP_LIBRARIES)
