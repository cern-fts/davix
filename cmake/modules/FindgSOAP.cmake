#
# This module detects if gsoap is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
#
# GSOAP_LIBRARIES       = full path to the gsoap libraries
# GSOAP_SSL_LIBRARIES   = full path to the gsoap ssl libraries
# GSOAP_INCLUDE_DIR     = include dir to be used when using the gsoap library
# GSOAP_WSDL2H          = wsdl2h binary
# GSOAP_SOAPCPP2        = soapcpp2 binary
# GSOAP_FOUND           = set to true if gsoap was found successfully
#
# GSOAP_LOCATION
#   setting this enables search for gsoap libraries / headers in this location


# -----------------------------------------------------
# GSOAP Libraries
# -----------------------------------------------------
find_library(GSOAP_LIBRARIES
    NAMES gsoap
    HINTS ${GSOAP_LOCATION}/lib ${GSOAP_LOCATION}/lib64
          ${GSOAP_LOCATION}/lib32
    DOC "The main gsoap library"
)
find_library(GSOAP_SSL_LIBRARIES
    NAMES gsoapssl
    HINTS ${GSOAP_LOCATION}/lib ${GSOAP_LOCATION}/lib64
          ${GSOAP_LOCATION}/lib32
    DOC "The ssl gsoap library"
)

# -----------------------------------------------------
# GSOAP Include Directories
# -----------------------------------------------------
find_path(GSOAP_INCLUDE_DIR
    NAMES stdsoap2.h
    HINTS ${GSOAP_LOCATION} ${GSOAP_LOCATION}/include ${GSOAP_LOCATION}/include/*
    DOC "The gsoap include directory"
)

# -----------------------------------------------------
# GSOAP Binaries
# -----------------------------------------------------
find_program(GSOAP_WSDL2H
    NAMES wsdl2h
    HINTS ${GSOAP_LOCATION}/bin
    DOC "The gsoap bin directory"
)
find_program(GSOAP_SOAPCPP2
    NAMES soapcpp2
    HINTS ${GSOAP_LOCATION}/bin
    DOC "The gsoap bin directory"
)

# -----------------------------------------------------
# GSOAP_276_COMPAT_FLAGS and GSOAPVERSION
# try to determine the flagfor the 2.7.6 compatiblity, break with 2.7.13 and re-break with 2.7.16
# ----------------------------------------------------
message(STATUS " - wsdlh : ${GSOAP_WSDL2H}")
message(STATUS " - SOAPCPP2 : ${GSOAP_SOAPCPP2}")

execute_process(COMMAND ${GSOAP_SOAPCPP2}  "-help"   OUTPUT_VARIABLE GSOAP_STRING_VERSION ERROR_VARIABLE GSOAP_STRING_VERSION )
string(REGEX MATCH "[0-9]*\\.[0-9]*\\.[0-9]*" GSOAP_VERSION ${GSOAP_STRING_VERSION})

if( "${GSOAP_VERSION}" STREQUAL "..")
  execute_process(COMMAND ${GSOAP_SOAPCPP2}  "-v" TIMEOUT 1 OUTPUT_VARIABLE GSOAP_STRING_VERSION ERROR_VARIABLE GSOAP_STRING_VERSION )
  string(REGEX MATCH "[0-9]*\\.[0-9]*\\.[0-9]*" GSOAP_VERSION ${GSOAP_STRING_VERSION})
endif()

message(STATUS " - GSOAP VERSION : ${GSOAP_VERSION}")
if( "${GSOAP_VERSION}"  VERSION_LESS "2.7.6")
	set(GSOAP_276_COMPAT_FLAGS "")
elseif ( "${GSOAP_VERSION}"  VERSION_LESS "2.7.14")
	set(GSOAP_276_COMPAT_FLAGS "-z")
else ( "${GSOAP_VERSION}"  VERSION_LESS "2.7.14")
	set(GSOAP_276_COMPAT_FLAGS "-z1 -z2")
endif ( "${GSOAP_VERSION}"  VERSION_LESS "2.7.6")



# -----------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set GSOAP_FOUND to TRUE if
# all listed variables are TRUE
# -----------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(gsoap DEFAULT_MSG GSOAP_LIBRARIES
    GSOAP_INCLUDE_DIR GSOAP_WSDL2H GSOAP_SOAPCPP2)
mark_as_advanced(GSOAP_INCLUDE_DIR GSOAP_LIBRARIES GSOAP_WSDL2H GSOAP_SOAPCPP2)
