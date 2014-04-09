# main file for src


FILE(GLOB src_davix "*.cpp")

FILE(GLOB src_neon "neon/*.cpp")
FILE(GLOB src_davix_posix "posix/*.cpp")
FILE(GLOB src_davix_status "status/*.cpp")
FILE(GLOB src_davix_fileops "fileops/*.cpp")
FILE(GLOB src_davix_params "params/*.cpp")
FILE(GLOB src_davix_auth "auth/*.cpp")
FILE(GLOB src_davix_file "file/*.cpp")
FILE(GLOB src_davix_deprecated "deprecated/*.cpp")
FILE(GLOB src_davix_request "request/*.cpp")
FILE(GLOB src_davix_utils "utils/*.cpp")


include_directories("libs" ".")

configure_file(config.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
link_directories(${HTTPLIB_PKG_LIBRARY_DIR} ${CRYPTO_PKG_LIBRARY_DIR} 
                                        ${LIBXML2_PKG_LIBRARY_DIR} )

add_subdirectory(copy)
add_subdirectory(libs)
add_subdirectory(xml)
add_subdirectory(tools)

add_subdirectory(neon)
SET(SRC_HTTP ${src_neon})

LIST(APPEND SRC_DAVIX_FULL ${src_davix} ${src_davix_request} ${SRC_HTTP}  ${src_xmlparser} ${src_logger} ${src_datetime}
                           ${src_lockers} ${src_davix_posix} ${src_davix_status} ${src_davix_fileops} ${src_davix_utils}
                           ${src_davix_params} ${src_davix_auth} ${src_davix_file} ${src_davix_deprecated} ${SRC_STRPTIME} ${SRC_MEMPCPY}
                           ${SRC_STRLCPY} ${SRC_STRING_UTILS_CPP} ${SRC_BASE64} ${SRC_HMACSHA1})


add_definitions(${HTTPLIB_PKG_CFLAGS} )

if(shared_library)
add_library(libdavix  SHARED ${SRC_DAVIX_FULL})
add_dependencies(libdavix neon)


target_link_libraries(libdavix  ${HTTPLIB_PKG_LIBRARIES} ${CRYPTO_PKG_LIBRARIES}
                                ${CMAKE_THREAD_LIBS_INIT} ${SOCKET_PKG_LIBRARIES} ${Boost_LIBRARIES})


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
                               DESCRIPTION "Libdavix, IO library for HTTP based protocols"
                               )

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

SET(davix_tool_common_src "${davix_tool_common_src_up}" PARENT_SCOPE)


#install pkgconfig
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/davix.pc
        DESTINATION ${PKGCONFIG_FILES_DIR} )


