#
# This module detects if W32 SOCKETS are needed 
# include files and libraries are.
#
# This code sets the following variables:
# 
# SOCKET_PKG_LIBRARIES   =  link flag for the w32 sockets


if(MSYS)
set(SOCKET_PKG_LIBRARIES "ws2_32")
set(SOCKET_PKG_FOUND TRUE)
# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set LFC_FOUND to TRUE if
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SOCKET_PKG DEFAULT_MSG SOCKET_PKG_LIBRARIES)
mark_as_advanced(SOCKET_PKG_LIBRARIES)

endif(MSYS)




