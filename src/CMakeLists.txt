# main file for src


FILE(GLOB src_davix "*.cpp")

FILE(GLOB src_neon "neon/*.cpp")
FILE(GLOB src_logger "logging/*.c")

LIST(APPEND davix_headers "davix.h" "davix_types.h" "composition.h" "global_def.h"
            "libdavix_object.h" "abstractsessionfactory.h" "request.h" "httprequest.h" "davix_cpp.hpp")

LIST(APPEND davix_logger_header "logging/logger.h")

include_directories("libs")

add_subdirectory(libs)
add_subdirectory(datetime)
add_subdirectory(logging)
add_subdirectory(xmlpp)
if ( ${HTTP_LIB} STREQUAL "NEON" )

    add_subdirectory(neon)
    SET(SRC_HTTP ${src_neon})

else ( ${HTTP_LIB} STREQUAL "NEON" )

    add_subdirectory(curl)
    SET(SRC_HTTP ${src_curl})

endif ( ${HTTP_LIB} STREQUAL "NEON" )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(${GLIBMM_PKG_CFLAGS} ${HTTPLIB_PKG_CFLAGS} ${LIBXMLPP_PKG_CFLAGS})
add_library(davix  SHARED ${src_davix} ${SRC_HTTP}  ${src_xmlparser} ${src_logger} ${src_datetime} ${src_crypto})


target_link_libraries(davix ${GLIBMM_PKG_LIBRARIES} ${HTTPLIB_PKG_LIBRARIES} ${CRYPTO_PKG_LIBRARIES} ${GTHREAD_PKG_LIBRARIES})

target_link_libraries(davix  ${LIBXMLPP_PKG_LIBRARIES})


set_target_properties(davix PROPERTIES
                            VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
                            SOVERSION ${VERSION_MAJOR}
                            CLEAN_DIRECT_OUTPUT 1
                            OUTPUT_NAME ${OUTPUT_NAME_DAVIX})

add_PkgConfigFile_for_Library("davix.pc" davix
                               HEADER_DIRS "davix"
                               CFLAGS " -D_FILE_OFFSET_BITS=64 "
                               DESCRIPTION "Davix, POSIX-like access for Webdav"
                               REQUIRES "glib-2.0" "glibmm-2.4")

# install lib
install(TARGETS davix
        LIBRARY       DESTINATION ${LIB_INSTALL_DIR} )

#install pkgconfig
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/davix.pc
        DESTINATION ${PKGCONFIG_FILES_DIR} )

# install headers
install( FILES ${davix_headers}
         DESTINATION ${INCLUDE_INSTALL_DIR}/davix)

install( FILES ${davix_logger_header}
         DESTINATION ${INCLUDE_INSTALL_DIR}/davix/logging)
