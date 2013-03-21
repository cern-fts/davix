# main file for src


FILE(GLOB src_davix "*.cpp")

FILE(GLOB src_neon "neon/*.cpp")
FILE(GLOB src_logger "logger/*.c")
FILE(GLOB src_davix_posix "posix/*.cpp")
FILE(GLOB src_davix_status "status/*.cpp")
FILE(GLOB src_davix_fileops "fileops/*.cpp")
FILE(GLOB src_davix_params "params/*.cpp")
FILE(GLOB src_davix_auth "auth/*.cpp")
FILE(GLOB src_davix_file "file/*.cpp")
FILE(GLOB src_davix_request "request/*.cpp")

LIST(APPEND davix_headers "davix_types.h"  "davixcontext.hpp"
             "davix.hpp" "davixuri.hpp" )

LIST(APPEND davix_logger_header "logger/davix_logger.h")

LIST(APPEND davix_posix_header "posix/davposix.hpp")

LIST(APPEND davix_status_header "status/davixstatusrequest.hpp")

LIST(APPEND davix_params_header "params/davixrequestparams.hpp" )

LIST(APPEND davix_auth_header "auth/davixauth.hpp" "auth/davixx509cred.hpp" )

LIST(APPEND davix_file_header "file/davfile.hpp")

LIST(APPEND davix_request_header "request/httprequest.hpp" "request/httpcachetoken.hpp")


include_directories("libs" ".")

configure_file(config.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
link_directories(${HTTPLIB_PKG_LIBRARY_DIR} ${CRYPTO_PKG_LIBRARY_DIR} 
                                        ${LIBXML2_PKG_LIBRARY_DIR} )

add_subdirectory(libs)
add_subdirectory(datetime)
add_subdirectory(logger)
add_subdirectory(xml)
add_subdirectory(tools)

add_subdirectory(neon)
SET(SRC_HTTP ${src_neon})

LIST(APPEND SRC_DAVIX_FULL ${src_davix} ${src_davix_request} ${SRC_HTTP}  ${src_xmlparser} ${src_logger} ${src_datetime}
                           ${src_lockers} ${src_davix_posix} ${src_davix_status} ${src_davix_fileops}
                           ${src_davix_params} ${src_davix_auth} ${src_davix_file} ${SRC_STRPTIME} ${SRC_MEMPCPY}
                           ${SRC_STRLCPY} ${SRC_STRING_UTILS_CPP})


add_definitions(${HTTPLIB_PKG_CFLAGS} )

if(shared_library)
add_library(libdavix  SHARED ${SRC_DAVIX_FULL})
add_dependencies(libdavix neon_static)


target_link_libraries(libdavix  ${HTTPLIB_PKG_LIBRARIES} ${CRYPTO_PKG_LIBRARIES}
                                          ${CMAKE_THREAD_LIBS_INIT} ${SOCKET_PKG_LIBRARIES})



set_target_properties(libdavix PROPERTIES
                            VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
                            SOVERSION ${VERSION_MAJOR}
                            CLEAN_DIRECT_OUTPUT 1
                            OUTPUT_NAME ${OUTPUT_NAME_DAVIX})

install(TARGETS libdavix
             DESTINATION ${LIB_INSTALL_DIR} )

add_PkgConfigFile_for_Library("davix.pc" libdavix
                               HEADER_DIRS "davix"
                               CFLAGS " -D_FILE_OFFSET_BITS=64 "
                               DESCRIPTION "Davix, High level WebDav/HTTP library"
                               REQUIRES "glib-2.0")

endif(shared_library)



if(static_library)

add_library(libdavix_static  STATIC ${SRC_DAVIX_FULL})
add_dependencies(libdavix_static neon_static)


target_link_libraries(libdavix_static  ${HTTPLIB_PKG_LIBRARIES} ${CRYPTO_PKG_LIBRARIES}
                                          ${CMAKE_THREAD_LIBS_INIT} ${SOCKET_PKG_LIBRARIES})



set_target_properties(libdavix_static PROPERTIES
                            VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
                            SOVERSION ${VERSION_MAJOR}
                            CLEAN_DIRECT_OUTPUT 1
                            OUTPUT_NAME ${OUTPUT_NAME_DAVIX})

install(TARGETS libdavix_static
             DESTINATION ${LIB_INSTALL_DIR} )

endif(static_library)




# install lib


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

install( FILES ${davix_file_header}
         DESTINATION ${INCLUDE_INSTALL_DIR}/davix/file)

install( FILES ${davix_request_header}
         DESTINATION ${INCLUDE_INSTALL_DIR}/davix/request)
