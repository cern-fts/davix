#include <gridsite.h>
#include <status/davixstatusrequest.hpp>
#include <stdsoap2.h>
#include "soapH.h"

#include "../auth/davixx509cred_internal.hpp"

#include "DelegationSoapBinding.nsmap"

#include "copy_internal.hpp"

using namespace Davix;

/// Do the delegation
std::string davix_delegate(const std::string &urlpp,
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
  std::string ucert, ukey, passwd;
  if (!X509CredentialExtra::get_x509_info(params.getClientCertX509(), &ucert, &ukey, &passwd)) {
      DavixError::setupError(err, COPY_SCOPE, DAVIX_STATUS_DELEGATION_ERROR,
              std::string("Third party copy only supports PEM certificates"));
      return "";
  }

  const std::vector<std::string> &capathList = params.listCertificateAuthorityPath();
  std::string capath;
  if (capathList.size() > 0)
      capath = capathList[0];

  // TODO: Get from the environment or something
  lifetime = 12 * 60 * 60; // 12h

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
                              NULL, capath.c_str(), NULL) == 0) {
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
                              NULL, capath.c_str(), NULL) == 0) {
            soap_call_tns__putProxy(soap_put,
                                    url,
                                    "http://www.gridsite.org/namespaces/delegation-1",
                                    delegation_id, certtxt, putProxyResponse);
            if (soap_put->error) {
              // Could not PUT
              err_aux = snprintf(err_buffer, sizeof(err_buffer), "Could not PUT the proxy: ");
              soap_sprint_fault(soap_put, err_buffer + err_aux, sizeof(err_buffer) - err_aux);

              DavixError::setupError(err, COPY_SCOPE, DAVIX_STATUS_DELEGATION_ERROR,
                                     err_buffer);
            }
        }
        else { // soap_put ssl error
          err_aux = snprintf(err_buffer, sizeof(err_buffer), "Connection error on proxy put: ");
          soap_sprint_fault(soap_put, err_buffer + err_aux, sizeof(err_buffer) - err_aux);

          DavixError::setupError(err, COPY_SCOPE, DAVIX_STATUS_DELEGATION_ERROR,
                                 err_buffer);
        }

        soap_free(soap_put);
      }
      else {
        DavixError::setupError(err, COPY_SCOPE, DAVIX_STATUS_DELEGATION_ERROR,
                               std::string("Could not generate the proxy: ") + err_buffer);
      }
    }
    else { // Could not get ID
      err_aux = snprintf(err_buffer, sizeof(err_buffer), "Could not get proxy request: ");
      soap_sprint_fault(soap_get, err_buffer + err_aux, sizeof(err_buffer) - err_aux);

      DavixError::setupError(err, COPY_SCOPE, DAVIX_STATUS_DELEGATION_ERROR,
                             std::string("Could not get the delegation id: ") + err_buffer);
    }
  }
  else { // soap_get ssl error
    err_aux = snprintf(err_buffer, sizeof(err_buffer), "Could not connect to get the proxy request: ");
    soap_sprint_fault(soap_get, err_buffer + err_aux, sizeof(err_buffer) - err_aux);
    DavixError::setupError(err, COPY_SCOPE, DAVIX_STATUS_DELEGATION_ERROR,
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
