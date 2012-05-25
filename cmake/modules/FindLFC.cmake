#
# This module detects if LFC is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
# 
# LFC_LIBRARIES   = full path to the LFC libraries
# LFC_INCLUDE_DIR = include dir to be used when using the LFC library
# LFC_FOUND       = set to true if LFC was found successfully
#
# LFC_LOCATION
#   setting this enables search for LFC libraries / headers in this location


# -----------------------------------------------------
# LFC Libraries
# -----------------------------------------------------
find_library(LFC_LIBRARIES
    NAMES lfc lcgdm
    HINTS ${LFC_LOCATION}/lib ${LFC_LOCATION}/lib64 ${LFC_LOCATION}/lib32
    DOC "The main LFC library"
)

# -----------------------------------------------------
# LFC Include Directories
# -----------------------------------------------------
find_path(LFC_INCLUDE_DIR 
    NAMES lfc/lfc_api.h 
    HINTS ${LFC_LOCATION} ${LFC_LOCATION}/include ${LFC_LOCATION}/include/*
    DOC "The LFC include directory"
)
if(LFC_INCLUDE_DIR)
    message(STATUS "LFC includes found in ${LFC_INCLUDE_DIR}")
endif()



# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set LFC_FOUND to TRUE if 
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LFC DEFAULT_MSG LFC_LIBRARIES LFC_INCLUDE_DIR)
mark_as_advanced(LFC_INCLUDE_DIR LFC_LIBRARIES)
