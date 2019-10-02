# main file for src

set(DAVIX_SOURCES
  auth/davix_openssl.cpp
  auth/davixauth.cpp
  auth/davixx509cred_internal.hpp                        auth/davixx509cred.cpp

  backend/BackendRequest.hpp                             backend/BackendRequest.cpp
  backend/SessionFactory.hpp                             backend/SessionFactory.cpp

  core/ContentProvider.hpp                               core/ContentProvider.cpp
  core/RedirectionResolver.hpp                           core/RedirectionResolver.cpp
  core/SessionPool.hpp
                                                         deprecated/httpcachetoken.cpp
                                                         file/davfile.cpp
                                                         file/davposix.cpp
  fileops/azure_meta_ops.hpp
  fileops/AzureIO.hpp                                    fileops/AzureIO.cpp
  fileops/chain_factory.hpp                              fileops/chain_factory.cpp
  fileops/davix_reliability_ops.hpp                      fileops/davix_reliability_ops.cpp
  fileops/davmeta.hpp                                    fileops/davmeta.cpp
  fileops/fileutils.hpp                                  fileops/fileutils.cpp
  fileops/httpiochain.hpp                                fileops/httpiochain.cpp
  fileops/httpiovec.hpp                                  fileops/httpiovec.cpp
  fileops/iobuffmap.hpp                                  fileops/iobuffmap.cpp
  fileops/S3IO.hpp                                       fileops/S3IO.cpp
                                                         hooks/davix_hooks.cpp
  modules/modules_profiles.hpp                           modules/modules_profiles.cpp
  neon/neonrequest.hpp                                   neon/neonrequest.cpp
  neon/neonsession.hpp                                   neon/neonsession.cpp
  neon/neonsessionfactory.hpp                            neon/neonsessionfactory.cpp
                                                         params/davixrequestparams.cpp
                                                         request/httprequest.cpp
                                                         status/davixstatusrequest.cpp
  utils/checksum_extractor.hpp                           utils/checksum_extractor.cpp
  utils/CompatibilityHacks.hpp                           utils/CompatibilityHacks.cpp
                                                         utils/davix_azure_utils.cpp
  utils/davix_fileproperties.hpp
                                                         utils/davix_gcloud_utils.cpp
  utils/davix_logger_internal.hpp                        utils/davix_logger.cpp
                                                         utils/davix_misc_utils.cpp
  utils/davix_utils_internal.hpp                         utils/davix_s3_utils.cpp
                                                         utils/davixuri.cpp
  xml/azurepropparser.hpp                                xml/azurepropparser.cpp
  xml/davdeletexmlparser.hpp                             xml/davdeletexmlparser.cpp
  xml/davix_ptree.hpp                                    xml/davix_ptree.cpp
  xml/davpropxmlparser.hpp                               xml/davpropxmlparser.cpp
  xml/davxmlparser.hpp                                   xml/davxmlparser.cpp
  xml/metalinkparser.hpp                                 xml/metalinkparser.cpp
  xml/s3deleteparser.hpp                                 xml/s3deleteparser.cpp
  xml/S3MultiPartInitiationParser.hpp                    xml/S3MultiPartInitiationParser.cpp
  xml/s3propparser.hpp                                   xml/s3propparser.cpp;

  davixcontext.cpp
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

LIST(APPEND SRC_DAVIX_FULL ${DAVIX_SOURCES} ${src_datetime}
                           ${SRC_STRPTIME} ${SRC_MEMPCPY} ${SRC_STRLCPY} ${SRC_STRING_UTILS_CPP}
                           ${SRC_BASE64} ${SRC_HMACSHA1})


add_definitions(${HTTPLIB_PKG_CFLAGS} )

if(SHARED_LIBRARY)
add_library(libdavix  SHARED ${SRC_DAVIX_FULL} )
add_dependencies(libdavix neon)


target_link_libraries(libdavix  alibxx_core ${HTTPLIB_PKG_LIBRARIES} ${CRYPTO_PKG_LIBRARIES}
                                ${CMAKE_THREAD_LIBS_INIT} ${SOCKET_PKG_LIBRARIES} ${UUID_LIBRARIES})

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
