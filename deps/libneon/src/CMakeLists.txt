
add_library(LibNeonObjects OBJECT
  ne_207.c
  ne_acl3744.c
  ne_alloc.c
  ne_auth.c
  ne_basic.c
  ne_compress.c
  ne_dates.c
  ne_i18n.c
  ne_locks.c
  ne_md5.c
  ne_ntlm.c
  ne_oldacl.c
  ne_pkcs11.c
  ne_props.c
  ne_redirect.c
  ne_request.c
  ne_session.c
  ne_socket.c
  ne_socks.c
  ne_sspi.c
  ne_string.c
  ne_uri.c
  ne_utils.c
  ne_xml.c
  ne_xmlreq.c
  ne_openssl.c
)

target_include_directories(LibNeonObjects PRIVATE ${LIBXML2_INCLUDE_DIR})
