#ifndef DAVIX_COPY_INTERNAL_HPP
#define DAVIX_COPY_INTERNAL_HPP

#include <davix.hpp>

extern const std::string COPY_SCOPE;


// Internal struct to hold data
class Davix::DavixCopyInternal
{
public:
    DavixCopyInternal(Davix::Context& ctx, const Davix::RequestParams *params):
        context(ctx), parameters(params),
        perfCallback(NULL), perfCallbackUdata(NULL)
    {
    }

    void copy(const Davix::Uri &source, const Davix::Uri &destination,
              unsigned nstreams, Davix::DavixError **error);

    void setPerformanceCallback(DavixCopy::PerformanceCallback callback, void *udata);

protected:
    Davix::Context &context;
    const Davix::RequestParams *parameters;
    DavixCopy::PerformanceCallback perfCallback;
    void *perfCallbackUdata;

    void monitorPerformanceMarkers(Davix::HttpRequest *request, Davix::DavixError **error);

    // Delegation
    static std::string davix_delegate(const std::string &urlpp,
                               const Davix::RequestParams& params,
                               Davix::DavixError** err);

private:
    DavixCopyInternal(const DavixCopyInternal&);
    DavixCopyInternal& operator = (const DavixCopyInternal&);

};


#endif //DAVIX_COPY_INTERNAL_HPP
