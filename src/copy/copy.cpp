#include <cstdio>
#include <cstring>
#include <davix.hpp>
#include <sstream>
#include <unistd.h>
#include "copy_internal.hpp"

using namespace Davix;

const std::string COPY_SCOPE = "Davix::HttpThirdPartyCopy";

// Gets the real location of uri respect to ref
// uri can be absolute or relative to ref
static std::string _full_url(const std::string ref,
        const std::string& uri)
{
    std::string final;

    if (uri.substr(0, 7).compare("http://") == 0) {
        final = uri;
    }
    else if (uri.substr(0, 8).compare("https://") == 0) {
        final = uri;
    }
    else if (uri[0] == '/') {
        size_t colon = ref.find(':');
        size_t slash = std::string::npos;
        if (colon != std::string::npos)
            slash = ref.find('/', colon + 3);
        if (slash != std::string::npos) {
            std::string base = ref.substr(0, slash);
            final = base + uri;
        }
    }
    else {
        final = ref + uri;
    }

    return final;
}



// Same as _full_url, but it makes sure
// that the destination is https
static std::string _full_delegation_endpoint(const std::string& ref,
        const std::string& uri, DavixError** err)
{
    std::string final = _full_url(ref, uri);
    if (final.substr(7).compare("http://") == 0) {
        DavixError::setupError(err, COPY_SCOPE, StatusCode::OperationNonSupported,
                               std::string("Plain http can not be used for delegation: ") + uri);
        final.clear();
    }
    return final;
}



DavixCopy::DavixCopy(Context &c, const RequestParams *params): d_ptr(NULL)
{
    d_ptr = new DavixCopyInternal(c, params);
}



DavixCopy::~DavixCopy()
{
    delete d_ptr;
}



void DavixCopy::copy(const Uri &source, const Uri &destination,
        unsigned nstreams, DavixError **error)
{
    d_ptr->copy(source, destination, nstreams, error);
}



void DavixCopy::setPerformanceCallback(PerformanceCallback callback, void *udata)
{
    d_ptr->setPerformanceCallback(callback, udata);
}



void DavixCopyInternal::setPerformanceCallback(DavixCopy::PerformanceCallback callback,
        void *udata)
{
    perfCallback = callback;
    perfCallbackUdata = udata;
}



void DavixCopyInternal::copy(const Uri &src, const Uri &dst,
        unsigned nstreams, DavixError **error)
{
    std::string nextSrc(src.getString()), prevSrc(src.getString());
    std::string destination(dst.getString());
    std::string delegationEndpoint;
    DavixError *internalError = NULL;

    // nstreams as string
    char nstreamsStr[16];
    snprintf(nstreamsStr, sizeof(nstreamsStr), "%u", nstreams);

    // Need a copy so we can modify it
    Davix::RequestParams requestParams(parameters);
    requestParams.setTransparentRedirectionSupport(false);

    // Perform COPY hopping through redirections
    HttpRequest* request = NULL;
    do {
        nextSrc = _full_url(prevSrc, nextSrc);
        prevSrc = nextSrc;
        if (request) {
            request->discardBody(&internalError);
            if (!internalError)
                request->endRequest(&internalError);
            if (internalError) {
                DavixError::propagatePrefixedError(error, internalError, __func__);
                break;
            }
        }
        delete request;

        davix_logger(DAVIX_LOG_VERBOSE, "Hop: %s",
                     nextSrc.c_str());

        request = context.createRequest(nextSrc, &internalError);
        if (internalError) {
            DavixError::propagatePrefixedError(error, internalError, __func__);
            break;
        }

        request->setRequestMethod("COPY");
        request->addHeaderField("Destination", destination);
        request->addHeaderField("X-Number-Of-Streams", nstreamsStr);
        request->setParameters(requestParams);
        request->beginRequest(&internalError);
        if (internalError) {
            DavixError::propagatePrefixedError(error, internalError, __func__);
            break;
        }

        // If we get a X-Delegate-To, before continuing, delegate
        if (request->getAnswerHeader("X-Delegate-To", delegationEndpoint)) {
            delegationEndpoint = _full_delegation_endpoint(nextSrc,
                                                           delegationEndpoint,
                                                           &internalError);
            if (internalError) {
                DavixError::propagatePrefixedError(error, internalError, __func__);
                break;
            }

            davix_logger(DAVIX_LOG_VERBOSE, "Got delegation endpoint: %s",
                         delegationEndpoint.c_str());

            std::string dlg_id = davix_delegate(delegationEndpoint,
                                                requestParams, &internalError);
            if (internalError) {
                DavixError::propagatePrefixedError(error, internalError, __func__);
                break;
            }

            davix_logger(DAVIX_LOG_VERBOSE, "Got delegation ID %s",
                         dlg_id.c_str());

            dlg_id.clear();
        }


    } while (request->getAnswerHeader("Location", nextSrc));

    if (!*error) {
        int responseStatus = request->getRequestCode();
        if (responseStatus == 404) {
            DavixError::setupError(error, COPY_SCOPE, StatusCode::FileNotFound,
                                   "Could not COPY. File not found");
        }
        else if (responseStatus == 403) {
            DavixError::setupError(error, COPY_SCOPE, StatusCode::PermissionRefused,
                                   "Could not COPY. Permission denied.");
        }
        else if (responseStatus == 501) {
            DavixError::setupError(error, COPY_SCOPE, StatusCode::OperationNonSupported,
                                   "Could not COPY. The source service does not support it");
        }
        else if (responseStatus >= 300) {
            std::ostringstream msg;
            msg << "Could not COPY. Unknown error code: " << responseStatus;
            DavixError::setupError(error, COPY_SCOPE, StatusCode::UnknowError,
                                   msg.str());
        }
    }

    // Did we fail?
    if (*error)
        return;

    // Finished hopping
    std::string finalSource = nextSrc;

    // Just wait for it to finish
    monitorPerformanceMarkers(request, error);
    request->endRequest(error);
    delete request;
}



void DavixCopyInternal::monitorPerformanceMarkers(Davix::HttpRequest *request,
        Davix::DavixError **error)
{
    Davix::DavixError* daverr = NULL;
    char buffer[1024], *p;
    size_t line_len;

    PerformanceMarker holder;
    PerformanceData performance;
    time_t lastPerfCallback = time(NULL);

    while ((line_len = request->readLine(buffer, sizeof(buffer), &daverr)) >= 0
            && !daverr)
    {
        buffer[line_len] = '\0';
        // Skip heading whitespaces
        p = buffer;
        while (*p && p < buffer + sizeof(buffer) && isspace(*p))
            ++p;

        if (strncasecmp("Perf Marker", p, 11) == 0)
        {
            memset(&holder, 0, sizeof(holder));
        }
        else if (strncasecmp("Timestamp:", p, 10) == 0)
        {
            holder.latest = atol(p + 10);
        }
        else if (strncasecmp("Stripe Index:", p, 13) == 0)
        {
            holder.index = atoi(p + 13);
        }
        else if (strncasecmp("Stripe Bytes Transferred:", p, 25) == 0)
        {
            holder.transferred = atol(p + 26);
        }
        else if (strncasecmp("Total Stripe Count:", p, 19) == 0)
        {
            holder.count = atoi(p + 20);
        }
        else if (strncasecmp("End", p, 3) == 0)
        {
            performance.update(holder);
            time_t now = time(NULL);
            if (now - lastPerfCallback >= 1)
            {
                if (this->perfCallback)
                    this->perfCallback(performance, this->perfCallbackUdata);
                lastPerfCallback = now;
            }
        }
        else if (strncasecmp("success", p, 7) == 0)
        {
            break;
        }
        else if (strncasecmp("aborted", p, 7) == 0)
        {
            Davix::DavixError::setupError(error, COPY_SCOPE, StatusCode::Canceled,
                    "Transfer aborted in the remote end");
            break;
        }
        else if (strncasecmp("failed", p, 6) == 0)
        {
            Davix::DavixError::setupError(error, COPY_SCOPE, StatusCode::RemoteError,
                    std::string("Transfer failed: ") + p);
            break;
        }
        else
        {
            Davix::DavixError::setupError(error, COPY_SCOPE, StatusCode::SystemError,
                    std::string("Unexpected message from remote host: ") + p);
            break;
        }
    }
}




