cmake_minimum_required (VERSION 2.6)

IF (ENABLE_THIRD_PARTY_COPY)
    INCLUDE     (CheckLibraryExists)

    pkg_check_modules(GSOAP_SSL_PKG    REQUIRED "gsoapssl++")

    FIND_PACKAGE(gSOAP REQUIRED)
    MESSAGE(STATUS "Using gsoapssl flags ${GSOAP_SSL_PKG_CFLAGS}")

    # Compile the WSDL for 1.1.0
    ADD_CUSTOM_COMMAND (OUTPUT typemap.dat
                        COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/delegation/typemap.dat ${CMAKE_CURRENT_BINARY_DIR}/typemap.dat)

    ADD_CUSTOM_COMMAND (
        OUTPUT delegation1.h
        COMMAND ${GSOAP_WSDL2H} -j -s -I/usr/share/gsoap/import/ -o${CMAKE_CURRENT_BINARY_DIR}/delegation1.h ${CMAKE_CURRENT_SOURCE_DIR}/delegation/delegation-1.1.0.wsdl -qdelegation1
        DEPENDS typemap.dat)
    ADD_CUSTOM_COMMAND (
        OUTPUT delegation1C.cpp delegation1Client.cpp
        COMMAND ${GSOAP_SOAPCPP2} -L -C -x -w -I/usr/share/gsoap/import/ ${CMAKE_CURRENT_BINARY_DIR}/delegation1.h -pdelegation1 -qdelegation1
        DEPENDS delegation1.h)

    # Compile the WSDL for 2.0.0
    ADD_CUSTOM_COMMAND (
        OUTPUT delegation2.h
        COMMAND ${GSOAP_WSDL2H} -j -s -I/usr/share/gsoap/import/ -o${CMAKE_CURRENT_BINARY_DIR}/delegation2.h ${CMAKE_CURRENT_SOURCE_DIR}/delegation/delegation-2.0.0.wsdl -qdelegation2
        DEPENDS typemap.dat)
    ADD_CUSTOM_COMMAND (
        OUTPUT delegation2C.cpp delegation2Client.cpp
        COMMAND ${GSOAP_SOAPCPP2} -C -x -w -I/usr/share/gsoap/import/ ${CMAKE_CURRENT_BINARY_DIR}/delegation2.h -n
        DEPENDS delegation2.h)

    # Need to generate some utility functions in the global namespace
    ADD_CUSTOM_COMMAND (OUTPUT env.h
                        COMMAND touch env.h)
    ADD_CUSTOM_COMMAND (OUTPUT envC.cpp
                        COMMAND ${GSOAP_SOAPCPP2}  ${CMAKE_CURRENT_BINARY_DIR}/env.h -penv
                        DEPENDS env.h)

    # Includes
    INCLUDE_DIRECTORIES (
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_BINARY_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
        ${GSOAP_INCLUDE_DIR}
    )

    # link
    LINK_DIRECTORIES( ${DAVIX_PKG_LIBRARY_DIRS})
    ADD_DEFINITIONS(${GSOAP_SSL_PKG_CFLAGS})

    # Build third party copy as a static library
    FILE(GLOB src_copy "*.cpp")
    FILE(GLOB src_dlg "delegation/*.cpp")

    ADD_LIBRARY (davix_copy SHARED ${src_copy} ${src_dlg}
                           ${CMAKE_CURRENT_BINARY_DIR}/delegation1C.cpp
                           ${CMAKE_CURRENT_BINARY_DIR}/delegation1Client.cpp
                           ${CMAKE_CURRENT_BINARY_DIR}/delegation2C.cpp
                           ${CMAKE_CURRENT_BINARY_DIR}/delegation2Client.cpp
                           ${CMAKE_CURRENT_BINARY_DIR}/envC.cpp)

    TARGET_LINK_LIBRARIES(davix_copy libdavix
                                     ${GSOAP_SSL_PKG_LIBRARIES}
                                     ${LIBSSL_PKG_LIBRARIES})

    SET_TARGET_PROPERTIES(davix_copy PROPERTIES
                            VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
                            SOVERSION ${VERSION_MAJOR}
                            LINK_INTERFACE_LIBRARIES ""
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
