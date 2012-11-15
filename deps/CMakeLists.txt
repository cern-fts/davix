###
## compile dependencies of davix

IF(NEON_EMBEDED)

set(NEON_CONFIGURE_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/libneon/configure")
set(NEON_CONFIGURE_ARGS "--with-ssl=openssl")
set(NEON_CONFIGURE_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/libneon/configure")

set(CONFIGURE_RESULT_CODE "")
set(CONFIGURE_RESULT_MSG "")

execute_process(COMMAND "${NEON_CONFIGURE_COMMAND}" "${NEON_CONFIGURE_ARGS}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/libneon/"

                )

add_custom_command(TARGET neon_static
                PRE_BUILD
                "make"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/libneon/"
                COMMENT "neon-build"
                )

SET(HTTPLIB_PKG_VERSION "0.29.0" PARENT_SCOPE)
SET(HTTPLIB_PKG_STATIC_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libneon/src/.libs/libneon.a")
SET(HTTPLIB_PKG_LIBRARIES "-lgssapi_krb5 -lkrb5 -lk5crypto -lcom_err -lxml2 ${HTTPLIB_PKG_STATIC_LIB_PATH}")
SET(HTTPLIB_PKG_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/libneon/src/")


ENDIF(NEON_EMBEDED)
