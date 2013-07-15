cmake_minimum_required (VERSION 2.6)

IF (ENABLE_THIRD_PARTY_COPY)
    INCLUDE     (CheckLibraryExists)

    pkg_check_modules(GSOAP_SSL_PKG    REQUIRED "gsoapssl++")
    pkg_check_modules(GRIDSITE_SSL_PKG REQUIRED "gridsite-openssl>=1.7.25")
    
    FIND_PACKAGE(gSOAP REQUIRED)
    MESSAGE(STATUS "Using gsoapssl flags ${GSOAP_SSL_PKG_CFLAGS}")
    
    IF (NOT DEFINED DELEGATION_WSDL)
        SET (DELEGATION_WSDL "${CMAKE_CURRENT_SOURCE_DIR}/delegation-1.1.0.wsdl")
    ENDIF (NOT DEFINED DELEGATION_WSDL)
    
    # Compile the WSDL
    ADD_CUSTOM_COMMAND (OUTPUT typemap.dat
                        COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/typemap.dat ${CMAKE_CURRENT_BINARY_DIR}/typemap.dat)
    ADD_CUSTOM_COMMAND (OUTPUT delegation.h
                        COMMAND ${GSOAP_WSDL2H} -I/usr/share/gsoap/import/ -o${CMAKE_CURRENT_BINARY_DIR}/delegation.h ${DELEGATION_WSDL}
                        DEPENDS typemap.dat)
    ADD_CUSTOM_COMMAND (OUTPUT soapC.cpp soapClient.cpp
                        COMMAND ${GSOAP_SOAPCPP2} -I/usr/share/gsoap/import/ ${CMAKE_CURRENT_BINARY_DIR}/delegation.h
                        DEPENDS delegation.h)
                        
    # Includes
    INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR}
                         ${GRIDSITE_SSL_PKG_INCLUDE_DIRS}
                         ${GSOAP_INCLUDE_DIR})
                         
    # link
    LINK_DIRECTORIES( ${DAVIX_PKG_LIBRARY_DIRS})
    ADD_DEFINITIONS(${GSOAP_SSL_PKG_CFLAGS})
    
    # Build third party copy as a static library
    FILE(GLOB src_copy "*.cpp")
    
    ADD_LIBRARY (davix_copy SHARED ${src_copy}
                                   ${CMAKE_CURRENT_BINARY_DIR}/soapC.cpp
                                   ${CMAKE_CURRENT_BINARY_DIR}/soapClient.cpp)
                                   
    TARGET_LINK_LIBRARIES(davix_copy libdavix
                                     ${GSOAP_SSL_PKG_LIBRARIES}
                                     ${GRIDSITE_SSL_PKG_LIBRARIES})
                                     
    SET_TARGET_PROPERTIES(davix_copy PROPERTIES
                            VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
                            SOVERSION ${VERSION_MAJOR}
                            CLEAN_DIRECT_OUTPUT 1
                            OUTPUT_NAME "davix_copy")

    INSTALL(TARGETS davix_copy
             DESTINATION ${LIB_INSTALL_DIR})
             
    add_PkgConfigFile_for_Library("davix_copy.pc" davix_copy
                               HEADER_DIRS "davix"
                               CFLAGS " -D_FILE_OFFSET_BITS=64 "
                               DESCRIPTION "Davix, High level WebDav/HTTP library. Third party copy library."
                               )
                               
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/davix_copy.pc
            DESTINATION ${PKGCONFIG_FILES_DIR})

ENDIF (ENABLE_THIRD_PARTY_COPY)
