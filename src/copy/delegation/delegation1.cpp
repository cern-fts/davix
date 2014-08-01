#include <algorithm>
#include <gridsite.h>
#include <status/davixstatusrequest.hpp>
#include <stdsoap2.h>
#include "delegation1H.h"

#include "delegation.hpp"

#include "../auth/davixx509cred_internal.hpp"

SOAP_NMAC struct Namespace namespaces[] =
{
	{"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
	{"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
	{"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
	{"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
	{"tns", "http://www.gridsite.org/namespaces/delegation-1", NULL, NULL},
	{"tns2", "http://www.gridsite.org/namespaces/delegation-2", NULL, NULL},
	{NULL, NULL, NULL, NULL}
};

using namespace Davix;




// Gsoap does not use the Davix HTTP request system
// Hooks need to be triggered manually for the modules
// stupid ugly hack to fix this
void triggerHooks(Context & context, RequestParams & params){
    RequestPreRunHook preRun = context.getHook<RequestPreRunHook>();
    Uri u;
    HttpRequest tmp_req(context, u, NULL);
    if(preRun){
        // force the run of the hook on req + params
        preRun(params, tmp_req, u);
    }
}

/// Do the delegation
std::string DavixDelegation::delegate_v1(Context & context,
		const std::string &dlg_endpint, const RequestParams& _p,
		const std::string& ucred, const std::string& passwd,
		const std::string& capath,
		int lifetime, DavixError** err)
{
  std::string  delegation_id;
  std::string  reqtxt;
  char*        certtxt = NULL;

  struct soap                        *soap_get = NULL, *soap_put = NULL;
  delegation1::tns__getNewProxyReqResponse  getNewProxyReqResponse;
  delegation1::tns__putProxyResponse        putProxyResponse;
  const char* url = dlg_endpint.c_str();
  char        err_buffer[512];
  size_t      err_aux;

  RequestParams params(_p);
  triggerHooks(context, params);

  // Request a new delegation ID
  soap_get = soap_new();
  soap_get->keep_alive = 1;

  if (soap_ssl_client_context(soap_get, SOAP_SSL_DEFAULT, ucred.c_str(), passwd.c_str(),
                              ucred.c_str(), capath.c_str(), NULL) == 0) {
    delegation1::soap_call_tns__getNewProxyReq(soap_get,
                                  url,
                                  "http://www.gridsite.org/namespaces/delegation-1",
                                  getNewProxyReqResponse);

    if(soap_get->error == 0) {
      reqtxt        = getNewProxyReqResponse.getNewProxyReqReturn->proxyRequest;
      delegation_id = getNewProxyReqResponse.getNewProxyReqReturn->delegationID;

      // Generate proxy
      if (GRSTx509MakeProxyCert(&certtxt, stderr, (char*)reqtxt.c_str(),
                                (char*)ucred.c_str(), (char*)ucred.c_str(), lifetime) == GRST_RET_OK) {
        // Submit the proxy
        soap_put = soap_new();

        if (soap_ssl_client_context(soap_put, SOAP_SSL_DEFAULT, ucred.c_str(), passwd.c_str(),
                ucred.c_str(), capath.c_str(), NULL) == 0) {
            soap_call_tns__putProxy(soap_put,
                                    url,
                                    "http://www.gridsite.org/namespaces/delegation-1",
                                    (char*)delegation_id.c_str(), certtxt, putProxyResponse);
            if (soap_put->error) {
              // Could not PUT
              err_aux = snprintf(err_buffer, sizeof(err_buffer), "Could not PUT the proxy: ");
              soap_sprint_fault(soap_put, err_buffer + err_aux, sizeof(err_buffer) - err_aux);

              DavixError::setupError(err, DELEGATION_SCOPE, StatusCode::DelegationError,
                                     err_buffer);
            }
        }
        else { // soap_put ssl error
          err_aux = snprintf(err_buffer, sizeof(err_buffer), "Connection error on proxy put: ");
          soap_sprint_fault(soap_put, err_buffer + err_aux, sizeof(err_buffer) - err_aux);

          DavixError::setupError(err, DELEGATION_SCOPE, StatusCode::DelegationError,
                                 err_buffer);
        }

        soap_free(soap_put);
      }
      else {
        DavixError::setupError(err, DELEGATION_SCOPE, StatusCode::DelegationError,
                               std::string("Could not generate the proxy: ") + err_buffer);
      }
    }
    else { // Could not get ID
      err_aux = snprintf(err_buffer, sizeof(err_buffer), "Could not get proxy request: ");
      soap_sprint_fault(soap_get, err_buffer + err_aux, sizeof(err_buffer) - err_aux);

      DavixError::setupError(err, DELEGATION_SCOPE, StatusCode::DelegationError,
                             std::string("Could not get the delegation id: ") + err_buffer);
    }
  }
  else { // soap_get ssl error
    err_aux = snprintf(err_buffer, sizeof(err_buffer), "Could not connect to get the proxy request: ");
    soap_sprint_fault(soap_get, err_buffer + err_aux, sizeof(err_buffer) - err_aux);
    DavixError::setupError(err, DELEGATION_SCOPE, StatusCode::DelegationError,
                           std::string("Could not connect to the delegation endpoint: ") + err_buffer);
  }

  // Clean soap_get
  soap_free(soap_get);
  free(certtxt);

  // Return delegation ID
  return delegation_id;
}
