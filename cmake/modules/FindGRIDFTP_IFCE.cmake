#
# This module detects if GRIDFTP_IFCE is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# GRIDFTP_IFCE_LIBRARIES   = full path to the dpm libraries
# GRIDFTP_IFCE_INCLUDE_DIR = include dir to be used when using the dpm library
# GRIDFTP_IFCE_FOUND       = set to true if dpm was found successfully
#
# GRIDFTP_IFCE_LOCATION
#   setting this enables search for dpm libraries / headers in this location


# -----------------------------------------------------
# DPM Libraries
# -----------------------------------------------------
find_library(GRIDFTP_IFCE_LIBRARIES
    NAMES gridftp_ifce
    HINTS ${GRIDFTP_IFCE_LOCATION}/lib ${GRIDFTP_IFCE_LOCATION}/lib64 ${GRIDFTP_IFCE_LOCATION}/lib32
    DOC "The main gridftp_ifce library"
)



# -----------------------------------------------------
# GRIDFTP_IFCE Include Directories
# -----------------------------------------------------
find_path(GRIDFTP_IFCE_INCLUDE_DIR 
    NAMES gridftp-ifce.h
    HINTS ${GRIDFTP_IFCE_LOCATION} ${GRIDFTP_IFCE_LOCATION}/include ${GRIDFTP_IFCE_LOCATION}/include/*
    DOC "The gridftp-ifce.h include directory"
)
if(GRIDFTP_IFCE_INCLUDE_DIR)
    message(STATUS "gridftp_ifce includes found in ${GRIDFTP_IFCE_INCLUDE_DIR}")
endif()







# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set GRIDFTP_IFCE_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(gridftp_ifce DEFAULT_MSG GRIDFTP_IFCE_LIBRARIES GRIDFTP_IFCE_INCLUDE_DIR)
mark_as_advanced(GRIDFTP_IFCE_INCLUDE_DIR GRIDFTP_IFCE_LIBRARIES)



