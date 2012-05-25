#
# This module detects if DCAP is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# DCAP_LIBRARIES   = full path to the DCAP libraries
# DCAP_INCLUDE_DIR = include dir to be used when using the DCAP library
# DCAP_FOUND       = set to true if DCAP was found successfully
#
# DCAP_LOCATION
#   setting this enables search for DCAP libraries / headers in this location


# -----------------------------------------------------
# DCAP Libraries
# -----------------------------------------------------
find_library(DCAP_LIBRARIES
    NAMES dcap
    HINTS ${DCAP_LOCATION}/lib ${DCAP_LOCATION}/lib64 ${DCAP_LOCATION}/lib32  ${STAGE_DIR}/lib ${STAGE_DIR}/lib64
    DOC "The main DCAP library"
)



# -----------------------------------------------------
# DCAP Include Directories
# -----------------------------------------------------
find_path(DCAP_INCLUDE_DIR 
    NAMES dcap.h 
    HINTS ${DCAP_LOCATION} ${DCAP_LOCATION}/include ${DCAP_LOCATION}/include/*  ${STAGE_DIR}/include 
    DOC "The DCAP include directory"
)
if(DCAP_INCLUDE_DIR)
    message(STATUS "DCAP includes found in ${DCAP_INCLUDE_DIR}")
endif()



# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set DCAP_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DCAP DEFAULT_MSG DCAP_LIBRARIES DCAP_INCLUDE_DIR)
mark_as_advanced(DCAP_INCLUDE_DIR DCAP_LIBRARIES)

