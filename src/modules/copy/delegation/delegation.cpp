/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

#include <cstdlib>
#include <cstring>
#include <stdsoap2.h>

#include <sstream>

#include "delegation2H.h"
#include "delegation.hpp"
#include "auth/davixx509cred_internal.hpp"
#include <utils/davix_logger_internal.hpp>

using namespace Davix;

const std::string Davix::DELEGATION_SCOPE = "Davix::HttpThirdPartyCopy::Delegation";

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

// Timestamp from ASN1 representation
static int get_timestamp_from_asn1(ASN1_TIME* asn1)
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

// Remaining lifetime from the given certificate
static int get_cert_remaining_life(const std::string& cert)
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

// Puts into ucred the credentials paths
// Puts into capath the CA directory
// Returns true if the path should be removed at the end
// Sets err on error
bool DavixDelegation::get_credentials(const Davix::RequestParams& params,
		std::string& ucred, std::string&passwd, std::string& capath,
		int* lifetime, DavixError** err)
{
	X509Credential credentials = params.getClientCertX509();
	if (!credentials.hasCert()) {
		std::pair<authCallbackClientCertX509, void*> x509callback =
				params.getClientCertCallbackX509();

		if (!x509callback.first) {
			DavixError::setupError(err, DELEGATION_SCOPE,
					StatusCode::CredentialNotFound,
					"No callback set for getting credentials. Can not delegate");
			return false;
		}

		SessionInfo sess;

		x509callback.first(x509callback.second, sess, &credentials, err);
		if (err && *err)
			return false;
	}

	std::string ucert, ukey;
	if (!X509CredentialExtra::get_x509_info(credentials, &ucert, &ukey, &passwd)) {
		DavixError::setupError(err, DELEGATION_SCOPE,
				StatusCode::DelegationError,
				std::string("Third party copy only supports PEM certificates"));
		return false;
	}

	const std::vector<std::string> &capathList = params.listCertificateAuthorityPath();
	if (capathList.size() > 0)
		capath = capathList[0];

	// Delegation lifetime (in minutes!)
	int cert_remaining_life = get_cert_remaining_life(ucert);
	int delegation_max_life = 12 * 60; // 12 hours

	// Delegated proxy lifetime should be shorter than the current lifetime!
	*lifetime = std::min(cert_remaining_life, delegation_max_life) - 1;

	// Should at least remain one minute
	if (*lifetime <= 1) {
		DavixError::setupError(err, DELEGATION_SCOPE,
				StatusCode::DelegationError,
				std::string("The certificate expired or has less than two minutes left!"));
		return false;
	}

	// Cert and key need to be in the same file
	if (ucert == ukey) {
		ucred.assign(ucert);
		return false;
	} else {
		FILE *ifp, *ofp;
		int fd;
		int c;

		char* aux = strdup("/tmp/.XXXXXX");

		fd = mkstemp(aux);
		ofp = fdopen(fd, "w");

		ifp = fopen(ukey.c_str(), "r");
		while ((c = fgetc(ifp)) != EOF)
			fputc(c, ofp);
		fclose(ifp);

		ifp = fopen(ukey.c_str(), "r");
		while ((c = fgetc(ifp)) != EOF)
			fputc(c, ofp);
		fclose(ifp);

		fclose(ofp);

		ucred.assign(aux);
		free(aux);
		return true;
	}
}

// Guess the delegation version that should be used
static int get_delegation_version(const std::string& ucred, const std::string& passwd,
		const std::string& capath, const std::string& dlg_endpoint,
		DavixError** err)
{
    struct soap *soap_v;
    char err_buffer[512];
    int version = -1;

    soap_v = soap_new();

    if (soap_ssl_client_context(soap_v, SOAP_SSL_DEFAULT, ucred.c_str(), passwd.c_str(),
                                  ucred.c_str(), capath.c_str(), NULL) == 0) {
        delegation2::tns2__getInterfaceVersionResponse response;
        delegation2::soap_call_tns2__getInterfaceVersion(soap_v, dlg_endpoint.c_str(),
                "http://www.gridsite.org/namespaces/delegation-2", response);

        if (soap_v->error == 0) {
            version = atoi(response.getInterfaceVersionReturn);
        }
        else {
            // Assume version 1 (does not implement the version method)
            version = 1;
        }
    }
    else {
        soap_sprint_fault(soap_v, err_buffer, sizeof(err_buffer));
        DavixError::setupError(err, DELEGATION_SCOPE,
                StatusCode::DelegationError,
                std::string("Could not connect to the delegation endpoint: ") + err_buffer);
    }

    soap_done(soap_v);
    soap_free(soap_v);

	return version;
}

static void triggerHooks(Context & context, RequestParams & params){
    RequestPreRunHook preRun = context.getHook<RequestPreRunHook>();
    Uri u;
    HttpRequest tmp_req(context, u, NULL);
    if(preRun){
        // force the run of the hook on req + params
        preRun(params, tmp_req, u);
    }
}

// Try out all the given delegation endpoints, until one works. Error is reported
// iff all of them fail.
std::string DavixDelegation::delegate(Context & context, const std::vector<std::string> &endpoints,
    const Davix::RequestParams& params, Davix::DavixError** err) {

    DavixError::clearError(err);

    std::vector<std::string> errors;

    for(size_t i = 0; i < endpoints.size(); i++) {
        DavixError *delegateError = NULL;
        DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_GRID, "Trying out delegation endpoint: {}", endpoints[i]);

        std::string delegationId = DavixDelegation::delegate(context, endpoints[i], params, &delegateError);
        if(!delegationId.empty() && !delegateError) {
            // Success
            return delegationId;
        }

        if(delegateError) {
            errors.emplace_back(delegateError->getErrMsg());
            DavixError::clearError(&delegateError);
        }
    }

    // Error, all delegation endpoints are broken
    std::ostringstream ss;
    for(size_t i = 0; i < errors.size(); i++) {
        ss << i << ") " << errors[i] << ". ";
    }

    DavixError::setupError(err, DELEGATION_SCOPE, StatusCode::DelegationError, ss.str());
    return std::string();
}


// Perform delegation, abstracting the version that is running on the server
std::string DavixDelegation::delegate(Context & context, const std::string &dlg_endpoint,
		const RequestParams& _p, Davix::DavixError** err)
{
	std::string ucreds, capath, passwd;
	int lifetime;

	RequestParams params(_p);
	triggerHooks(context, params);

    get_credentials(params, ucreds, passwd, capath, &lifetime, err);
	if (*err)
		return std::string();

	// Initialize SSL
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();

	// Figure out the version
	int delegation_version = get_delegation_version(ucreds, passwd, capath, dlg_endpoint, err);
	if (*err)
		return std::string();

    DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_GRID, "Delegation version: {}", delegation_version);

    switch (delegation_version)
    {
        case 1:
            return delegate_v1(context, dlg_endpoint,
                    params, ucreds, passwd, capath, lifetime, err);
        case 2:
            return delegate_v2(context, dlg_endpoint,
                                params, ucreds, passwd, capath, lifetime, err);
        default: {
            std::ostringstream err_msg;
            err_msg << "Unknown delegation version: " << delegation_version;
            DavixError::setupError(err, DELEGATION_SCOPE,
                    StatusCode::DelegationError,
                    err_msg.str());
        }
    }

	return std::string();
}
