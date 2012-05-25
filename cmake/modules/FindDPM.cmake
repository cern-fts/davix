#
# This module detects if dpm is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# DPM_LIBRARIES   = full path to the dpm libraries
# DPM_INCLUDE_DIR = include dir to be used when using the dpm library
# DPM_FOUND       = set to true if dpm was found successfully
#
# DPM_LOCATION
#   setting this enables search for dpm libraries / headers in this location


# -----------------------------------------------------
# DPM Libraries
# -----------------------------------------------------
find_library(DPM_LIBRARIES
    NAMES dpm
    HINTS ${DPM_LOCATION}/lib ${DPM_LOCATION}/lib64 ${DPM_LOCATION}/lib32
    DOC "The main dpm library"
)


# -----------------------------------------------------
# LCGDM Libraries
# -----------------------------------------------------
find_library(LCGDM_LIBRARIES
    NAMES lcgdm
    HINTS ${DPM_LOCATION}/lib ${DPM_LOCATION}/lib64 ${DPM_LOCATION}/lib32
    DOC "The main lcgdm library"
)

# -----------------------------------------------------
# DPM Include Directories
# -----------------------------------------------------
find_path(DPM_INCLUDE_DIR 
    NAMES dpm/dpm_api.h 
    HINTS ${DPM_LOCATION} ${DPM_LOCATION}/include ${DPM_LOCATION}/include/*
    DOC "The dpm include directory"
)
if(DPM_INCLUDE_DIR)
    message(STATUS "dpm includes found in ${DPM_INCLUDE_DIR}")
endif()



find_path(LCGDM_INCLUDE_DIR 
	NAMES Cinit.h
	HINTS ${LCGDM_LOCATION} ${LCGDM_LOCATION}/include 
		  ${LCGDM_LOCATION}/include/lcgdm /usr/include/lcgdm 
		  ${LCGDM_LOCATION}/include/dpm /usr/include/dpm /usr/include/lfc
	DOC "The LCGDM include directory"
)



if(LCGDM_INCLUDE_DIR)
	message(STATUS "lcgdm includes found in ${LCGDM_INCLUDE_DIR}")
endif()





# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set DPM_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(dpm DEFAULT_MSG DPM_LIBRARIES DPM_INCLUDE_DIR)
mark_as_advanced(DPM_INCLUDE_DIR DPM_LIBRARIES)

find_package_handle_standard_args(lcgdm DEFAULT_MSG LCGDM_LIBRARIES LCGDM_INCLUDE_DIR)
mark_as_advanced(LCGDM_INCLUDE_DIR LCGDM_LIBRARIES)

