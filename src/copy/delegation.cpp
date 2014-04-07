#include <algorithm>
#include <gridsite.h>
#include <status/davixstatusrequest.hpp>
#include <stdsoap2.h>
#include "soapH.h"

#include "../auth/davixx509cred_internal.hpp"

#include "DelegationSoapBinding.nsmap"

#include "copy_internal.hpp"

using namespace Davix;

int get_timestamp_from_asn1(ASN1_TIME* asn1)
{
    char* data = (char*) ASN1_STRING_data(asn1);
    size_t len = strlen(data);
    struct tm time_tm;
    char zone = 0;

    memset(&time_tm, 0, sizeof(struct tm));

    if (len == 11
            && sscanf(data, "%02d%02d%02d%02d%02d%c", &(time_tm.tm_year),
                      &(time_tm.tm_mon), &(time_tm.tm_mday), &(time_tm.tm_hour),
                      &(time_tm.tm_min), &zone) != 7) {
    }
    else if (len == 13
            && sscanf(data, "%02d%02d%02d%02d%02d%02d%c", &(time_tm.tm_year),
                    &(time_tm.tm_mon), &(time_tm.tm_mday), &(time_tm.tm_hour),
                    &(time_tm.tm_min), &(time_tm.tm_sec), &zone) != 7) {
        return 0;
    }
    else if (len == 15
            && sscanf(data, "20%02d%02d%02d%02d%02d%02d%c", &(time_tm.tm_year),
                    &(time_tm.tm_mon), &(time_tm.tm_mday), &(time_tm.tm_hour),
                    &(time_tm.tm_min), &(time_tm.tm_sec), &zone) != 7) {
        return 0;
    }

    if (zone != 'Z')
        return 0;

    if (time_tm.tm_year < 90)
        time_tm.tm_year += 100;
    --(time_tm.tm_mon);

    return timegm(&time_tm);
}

int get_cert_remaining_life(const std::string& cert)
{
    FILE* f = fopen(cert.c_str(), "r");
    if (f == NULL)
        return 0;
    
    X509 *x509_cert = PEM_read_X509(f, NULL, NULL, NULL);
    if (x509_cert == NULL)
        return 0;
    
    ASN1_TIME* expiration = X509_get_notAfter(x509_cert);
    int expiration_timestamp = get_timestamp_from_asn1(expiration);
    X509_free(x509_cert);
    
    return (expiration_timestamp - time(NULL)) / 60;
}

/// Do the delegation
std::string DavixCopyInternal::davix_delegate(const std::string &urlpp,
                           const RequestParams& params,
                           DavixError** err)
{
  char                               *delegation_id = NULL;
  std::string                        *reqtxt  = NULL;
  char                               *certtxt = NULL;
  char                               *keycert = NULL;
  struct soap                        *soap_get = NULL, *soap_put = NULL;
  struct tns__getNewProxyReqResponse  getNewProxyReqResponse;
  struct tns__putProxyResponse        putProxyResponse;
  int                                 lifetime;
  const char* url = urlpp.c_str();
  char        err_buffer[512];
  size_t      err_aux;

  // Extract credentials
  std::pair<authCallbackClientCertX509, void*> x509callback = params.getClientCertCallbackX509();
  Davix::X509Credential credentials;

  if (!x509callback.first) {
      Davix::DavixError::setupError(err, COPY_SCOPE, StatusCode::CredentialNotFound,
                                    "No callback set for getting credentials. Can not delegate");
      return "";
  }

  SessionInfo sess;

  x509callback.first(x509callback.second, sess, &credentials, err);
  if (err && *err)
      return "";


  std::string ucert, ukey, passwd;
  if (!X509CredentialExtra::get_x509_info(credentials, &ucert, &ukey, &passwd)) {
      DavixError::setupError(err, COPY_SCOPE, StatusCode::DelegationError,
              std::string("Third party copy only supports PEM certificates"));
      return "";
  }

  const std::vector<std::string> &capathList = params.listCertificateAuthorityPath();
  std::string capath;
  if (capathList.size() > 0)
      capath = capathList[0];

  // Delegation lifetime (in minutes!)
  int cert_remaining_life = get_cert_remaining_life(ucert);
  int delegation_max_life = 12 * 60; // 12 hours
  
  // Delegated proxy lifetime should be shorter than the current lifetime!
  lifetime = std::min(cert_remaining_life, delegation_max_life) - 1;
  
  // Should at least remain one minute
  if (lifetime <= 1) {
      DavixError::setupError(err, COPY_SCOPE, StatusCode::DelegationError,
                             std::string("The certificate expired or has less than two minutes left!"));
      return "";
  }

  // Cert and key need to be in the same file
  if (ucert == ukey) {
    keycert = strdup(ucert.c_str());
  }
  else {
    FILE *ifp, *ofp;
    int   fd;
    int  c;

    keycert = strdup("/tmp/.XXXXXX");

    fd = mkstemp(keycert);
    ofp = fdopen(fd, "w");

    ifp = fopen(ukey.c_str(), "r");
    while ((c = fgetc(ifp)) != EOF) fputc(c, ofp);
    fclose(ifp);

    ifp = fopen(ukey.c_str(), "r");
    while ((c = fgetc(ifp)) != EOF) fputc(c, ofp);
    fclose(ifp);

    fclose(ofp);
  }

  // Initialize SSL
  ERR_load_crypto_strings ();
  OpenSSL_add_all_algorithms();

  // Request a new delegation ID
  soap_get = soap_new();
  soap_get->keep_alive = 1;

  if (soap_ssl_client_context(soap_get, SOAP_SSL_DEFAULT, keycert, passwd.c_str(),
                              ucert.c_str(), capath.c_str(), NULL) == 0) {
    soap_call_tns__getNewProxyReq(soap_get,
                                  url,
                                  "http://www.gridsite.org/namespaces/delegation-1",
                                  getNewProxyReqResponse);

    if(soap_get->error == 0) {
      reqtxt        = getNewProxyReqResponse.getNewProxyReqReturn->proxyRequest;
      delegation_id = strdup(getNewProxyReqResponse.getNewProxyReqReturn->delegationID->c_str());

      // Generate proxy
      if (GRSTx509MakeProxyCert(&certtxt, stderr, (char*)reqtxt->c_str(),
                                (char*)ucert.c_str(), (char*)ukey.c_str(), lifetime) == GRST_RET_OK) {
        // Submit the proxy
        soap_put = soap_new();

        if (soap_ssl_client_context(soap_put, SOAP_SSL_DEFAULT, keycert, "",
                ucert.c_str(), capath.c_str(), NULL) == 0) {
            soap_call_tns__putProxy(soap_put,
                                    url,
                                    "http://www.gridsite.org/namespaces/delegation-1",
                                    delegation_id, certtxt, putProxyResponse);
            if (soap_put->error) {
              // Could not PUT
              err_aux = snprintf(err_buffer, sizeof(err_buffer), "Could not PUT the proxy: ");
              soap_sprint_fault(soap_put, err_buffer + err_aux, sizeof(err_buffer) - err_aux);

              DavixError::setupError(err, COPY_SCOPE, StatusCode::DelegationError,
                                     err_buffer);
            }
        }
        else { // soap_put ssl error
          err_aux = snprintf(err_buffer, sizeof(err_buffer), "Connection error on proxy put: ");
          soap_sprint_fault(soap_put, err_buffer + err_aux, sizeof(err_buffer) - err_aux);

          DavixError::setupError(err, COPY_SCOPE, StatusCode::DelegationError,
                                 err_buffer);
        }

        soap_free(soap_put);
      }
      else {
        DavixError::setupError(err, COPY_SCOPE, StatusCode::DelegationError,
                               std::string("Could not generate the proxy: ") + err_buffer);
      }
    }
    else { // Could not get ID
      err_aux = snprintf(err_buffer, sizeof(err_buffer), "Could not get proxy request: ");
      soap_sprint_fault(soap_get, err_buffer + err_aux, sizeof(err_buffer) - err_aux);

      DavixError::setupError(err, COPY_SCOPE, StatusCode::DelegationError,
                             std::string("Could not get the delegation id: ") + err_buffer);
    }
  }
  else { // soap_get ssl error
    err_aux = snprintf(err_buffer, sizeof(err_buffer), "Could not connect to get the proxy request: ");
    soap_sprint_fault(soap_get, err_buffer + err_aux, sizeof(err_buffer) - err_aux);
    DavixError::setupError(err, COPY_SCOPE, StatusCode::DelegationError,
                           std::string("Could not connect to the delegation endpoint: ") + err_buffer);
  }

  // Clean soap_get
  soap_free(soap_get);
  free(keycert);
  free(certtxt);

  // Return delegation ID
  std::string did;
  if (delegation_id) {
      did.assign(delegation_id);
      free(delegation_id);
  }
  return did;
}
