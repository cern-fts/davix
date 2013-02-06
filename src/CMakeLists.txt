# main file for src


FILE(GLOB src_davix "*.cpp")

FILE(GLOB src_neon "neon/*.cpp")
FILE(GLOB src_logger "logger/*.c")
FILE(GLOB src_davix_posix "posix/*.cpp")
FILE(GLOB src_davix_status "status/*.cpp")
FILE(GLOB src_davix_fileops "fileops/*.cpp")
FILE(GLOB src_davix_params "params/*.cpp")
FILE(GLOB src_davix_auth "auth/*.cpp")

LIST(APPEND davix_headers "davix.h" "davix_types.h"  "davixuri.h" "davixcontext.hpp" "global_def.hpp"
            "abstractsessionfactory.hpp" "httprequest.hpp" "davix.hpp" "davixuri.hpp" )

LIST(APPEND davix_logger_header "logger/davix_logger.h")

LIST(APPEND davix_posix_header "posix/davix_posix.h" "posix/davposix.hpp")

LIST(APPEND davix_status_header "status/davixstatusrequest.hpp" "status/davix_error.h")

LIST(APPEND davix_params_header "params/davixrequestparams.hpp" "params/davixrequestparams.h")

LIST(APPEND davix_auth_header "auth/davixauth.hpp" "auth/davixx509cred.hpp" "auth/davixauth.h" "auth/davixx509cred.h")


include_directories("libs" ".")

add_subdirectory(libs)
add_subdirectory(datetime)
add_subdirectory(logger)
add_subdirectory(xml)
add_subdirectory(tools)

add_subdirectory(neon)
SET(SRC_HTTP ${src_neon})


include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(${HTTPLIB_PKG_CFLAGS} )
add_library(libdavix  SHARED ${src_davix} ${SRC_HTTP}  ${src_xmlparser} ${src_logger} ${src_datetime}
                           ${src_lockers} ${src_davix_posix} ${src_davix_status} ${src_davix_fileops}
                           ${src_davix_params} ${src_davix_auth})
add_dependencies(libdavix neon_static)


target_link_libraries(libdavix  ${HTTPLIB_PKG_LIBRARIES} ${CRYPTO_PKG_LIBRARIES} ${GTHREAD_PKG_LIBRARIES} ${GLIB2_PKG_LIBRARIES} )



set_target_properties(libdavix PROPERTIES
                            VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
                            SOVERSION ${VERSION_MAJOR}
                            CLEAN_DIRECT_OUTPUT 1
                            OUTPUT_NAME ${OUTPUT_NAME_DAVIX})

add_PkgConfigFile_for_Library("davix.pc" libdavix
                               HEADER_DIRS "davix"
                               CFLAGS " -D_FILE_OFFSET_BITS=64 "
                               DESCRIPTION "Davix, High level WebDav/HTTP library"
                               REQUIRES "glib-2.0")

# install lib
install(TARGETS libdavix
        LIBRARY       DESTINATION ${LIB_INSTALL_DIR} )

#install pkgconfig
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/davix.pc
        DESTINATION ${PKGCONFIG_FILES_DIR} )

# install headers
install( FILES ${davix_headers}
         DESTINATION ${INCLUDE_INSTALL_DIR}/davix)

install( FILES ${davix_logger_header}
         DESTINATION ${INCLUDE_INSTALL_DIR}/davix/logger)

install( FILES ${davix_posix_header}
         DESTINATION ${INCLUDE_INSTALL_DIR}/davix/posix)


install( FILES ${davix_status_header}
         DESTINATION ${INCLUDE_INSTALL_DIR}/davix/status)

install( FILES ${davix_params_header}
         DESTINATION ${INCLUDE_INSTALL_DIR}/davix/params)


install( FILES ${davix_auth_header}
         DESTINATION ${INCLUDE_INSTALL_DIR}/davix/auth)
