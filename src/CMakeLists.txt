# main file for src

FILE(GLOB src_davix
    "*.cpp" "neon/*.cpp" "status/*.cpp" "fileops/*.cpp"
    "params/*.cpp" "auth/*.cpp" "file/*.cpp" "deprecated/*.cpp"
    "request/*.cpp" "hooks/*.cpp" "modules/*.cpp" "utils/*.cpp" "xml/*.cpp"
)


include_directories("libs" ".")

configure_file(davix_internal_config.in ${CMAKE_CURRENT_BINARY_DIR}/davix_internal_config.hpp @ONLY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
link_directories(${HTTPLIB_PKG_LIBRARY_DIR} ${CRYPTO_PKG_LIBRARY_DIR}
                                        ${LIBXML2_PKG_LIBRARY_DIR} )

add_subdirectory(modules/copy)
add_subdirectory(libs)

if(ENABLE_TOOLS)
  add_subdirectory(tools)
endif()

LIST(APPEND SRC_DAVIX_FULL ${src_davix} ${src_datetime}
                           ${SRC_STRPTIME} ${SRC_MEMPCPY} ${SRC_STRLCPY} ${SRC_STRING_UTILS_CPP}
                           ${SRC_BASE64} ${SRC_HMACSHA1})


add_definitions(${HTTPLIB_PKG_CFLAGS} )

if(SHARED_LIBRARY)
add_library(libdavix  SHARED ${SRC_DAVIX_FULL} )
add_dependencies(libdavix neon)


target_link_libraries(libdavix  alibxx_core ${HTTPLIB_PKG_LIBRARIES} ${CRYPTO_PKG_LIBRARIES}
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
#install pkgconfig
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/davix.pc
        DESTINATION ${PKGCONFIG_FILES_DIR} )

endif(SHARED_LIBRARY)



if(STATIC_LIBRARY)

add_definitions(-DA_LIB_NAMESPACE=Davix)
FILE(GLOB src_alibxx_static "libs/alibxx/*/*.cpp")
add_library(libdavix_static  STATIC ${SRC_DAVIX_FULL} ${src_alibxx_static})

set_target_properties(libdavix_static PROPERTIES
                            OUTPUT_NAME ${OUTPUT_NAME_DAVIX})


install(TARGETS libdavix_static
             DESTINATION ${LIB_INSTALL_DIR} )

endif(STATIC_LIBRARY)

SET(davix_tool_common_src "${davix_tool_common_src_up}" PARENT_SCOPE)



