###
## compile dependencies of davix

IF(NEON_EMBEDED)


SET(HTTPLIB_PKG_VERSION "0.29.0" PARENT_SCOPE)
SET(HTTPLIB_PKG_LIBRARIES "-lgssapi_krb5 -lkrb5 -lk5crypto -lcom_err -lxml2")
SET(HTTPLIB_PKG_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/libneon/src/")

ENDIF(NEON_EMBEDED)
