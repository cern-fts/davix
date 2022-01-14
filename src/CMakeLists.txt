# main file for src

set(DAVIX_SOURCES

  auth/davix_openssl.cpp
  auth/davixauth.cpp
  auth/davixx509cred_internal.hpp                        auth/davixx509cred.cpp

  backend/BackendRequest.hpp                             backend/BackendRequest.cpp
  backend/SessionFactory.hpp                             backend/SessionFactory.cpp
  backend/StandaloneNeonRequest.hpp                      backend/StandaloneNeonRequest.cpp

  core/ContentProvider.hpp                               core/ContentProvider.cpp
  core/RedirectionResolver.hpp                           core/RedirectionResolver.cpp
  core/SessionPool.hpp

  curl/CurlSession.hpp                                   curl/CurlSession.cpp
  curl/CurlSessionFactory.hpp                            curl/CurlSessionFactory.cpp
  curl/HeaderlineParser.hpp                              curl/HeaderlineParser.cpp
  curl/ResponseBuffer.hpp                                curl/ResponseBuffer.cpp
  curl/StandaloneCurlRequest.hpp                         curl/StandaloneCurlRequest.cpp

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
  fileops/SwiftIO.hpp                                    fileops/SwiftIO.cpp

                                                         hooks/davix_hooks.cpp

                                                         libs/alibxx/chrono/timepoint.cpp
                                                         libs/alibxx/crypto/base64.cpp
                                                         libs/alibxx/crypto/hmacsha.cpp
                                                         libs/alibxx/str/format.cpp

  libs/datetime/datetime_utils.hpp                       libs/datetime/datetime_utils.cpp

  modules/modules_profiles.hpp                           modules/modules_profiles.cpp
  neon/neonrequest.hpp                                   neon/neonrequest.cpp
  neon/neonsession.hpp                                   neon/neonsession.cpp
  neon/neonsessionfactory.hpp                            neon/neonsessionfactory.cpp
                                                         params/davixrequestparams.cpp
                                                         request/httprequest.cpp
  status/DavixStatus.hpp                                 status/DavixStatus.cpp
                                                         status/davixstatusrequest.cpp

  utils/checksum_extractor.hpp                           utils/checksum_extractor.cpp
  utils/CompatibilityHacks.hpp                           utils/CompatibilityHacks.cpp
                                                         utils/davix_azure_utils.cpp
  utils/davix_fileproperties.hpp
                                                         utils/davix_gcloud_utils.cpp
  utils/davix_logger_internal.hpp                        utils/davix_logger.cpp
                                                         utils/davix_misc_utils.cpp
  utils/davix_utils_internal.hpp                         utils/davix_s3_utils.cpp
                                                         utils/davix_swift_utils.cpp
  utils/simple_get_pass.h                                utils/simple_get_pass.cpp
  utils/stringutils.hpp                                  utils/stringutils.cpp

                                                         utils/davixuri.cpp
  xml/azurepropparser.hpp                                xml/azurepropparser.cpp
  xml/davdeletexmlparser.hpp                             xml/davdeletexmlparser.cpp
  xml/davix_ptree.hpp                                    xml/davix_ptree.cpp
  xml/davpropxmlparser.hpp                               xml/davpropxmlparser.cpp
  xml/davxmlparser.hpp                                   xml/davxmlparser.cpp
  xml/metalinkparser.hpp                                 xml/metalinkparser.cpp
  xml/s3deleteparser.hpp                                 xml/s3deleteparser.cpp
  xml/S3MultiPartInitiationParser.hpp                    xml/S3MultiPartInitiationParser.cpp
  xml/s3propparser.hpp                                   xml/s3propparser.cpp
  xml/swiftpropparser.hpp                                xml/swiftpropparser.cpp

  davixcontext.cpp
  $<TARGET_OBJECTS:LibNeonObjects>
)

set(DAVIX_INTERNAL_INCLUDES
  ${OPENSSL_INCLUDE_DIR}
  ${LIBXML2_INCLUDE_DIR}
  ${PROJECT_SOURCE_DIR}/deps
  ${PROJECT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_BINARY_DIR}
)

configure_file(davix_internal_config.in ${CMAKE_CURRENT_BINARY_DIR}/davix_internal_config.hpp @ONLY)

add_subdirectory(modules/copy)

if(ENABLE_TOOLS)
  add_subdirectory(tools)
endif()

set(SECURE_TRANSPORT_LIBRARIES "")
if(APPLE)
  set(SECURE_TRANSPORT_LIBRARIES "-framework CoreFoundation -framework Security")
endif()

if(SHARED_LIBRARY)
add_library(libdavix  SHARED ${DAVIX_SOURCES} )

target_include_directories(libdavix PRIVATE ${DAVIX_INTERNAL_INCLUDES} )
target_link_libraries(libdavix
  PRIVATE ${libcurl}
  PUBLIC ${LIBSSL_PKG_LIBRARIES}
         ${LIBXML2_LIBRARIES}
         ${CMAKE_THREAD_LIBS_INIT}
         ${CMAKE_DL_LIBS}
         ${UUID_LIBRARIES}
         z
         ${SECURE_TRANSPORT_LIBRARIES}
)

set_target_properties(libdavix PROPERTIES
                            VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
                            SOVERSION ${VERSION_MAJOR}
                            LINK_INTERFACE_LIBRARIES ""
                            CLEAN_DIRECT_OUTPUT 1
                            OUTPUT_NAME ${OUTPUT_NAME_DAVIX})

install(TARGETS libdavix DESTINATION ${LIB_INSTALL_DIR} )

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

add_library(libdavix_static  STATIC ${DAVIX_SOURCES})
set_target_properties(libdavix_static PROPERTIES
                            OUTPUT_NAME ${OUTPUT_NAME_DAVIX})
target_include_directories(libdavix_static PRIVATE ${DAVIX_INTERNAL_INCLUDES} )

target_link_libraries(libdavix_static
  PRIVATE ${libcurl}
  PUBLIC  ${LIBSSL_PKG_LIBRARIES}
          ${LIBXML2_LIBRARIES}
          ${CMAKE_THREAD_LIBS_INIT}
          ${CMAKE_DL_LIBS}
          ${UUID_LIBRARIES}
          z
          ${SECURE_TRANSPORT_LIBRARIES}
)


add_library(libdavix ALIAS libdavix_static)

install(TARGETS libdavix_static DESTINATION ${LIB_INSTALL_DIR} )

endif(STATIC_LIBRARY)
