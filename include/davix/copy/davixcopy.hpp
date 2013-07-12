#ifndef DAVIX_COPY_HPP
#define DAVIX_COPY_HPP

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif

#include <vector>


namespace Davix {

struct DAVIX_EXPORT PerformanceMarker
{
    int    index, count;

    time_t begin, latest;
    off_t  transferred;

    off_t transferAvg;
    off_t transferInstant;

    PerformanceMarker();
};

struct DAVIX_EXPORT PerformanceData
{
    time_t begin, latest;

    std::vector<PerformanceMarker> markers;

    PerformanceData();
    ~PerformanceData();

    void update(const PerformanceMarker& in);

    time_t absElapsed() const;

    off_t avgTransfer(void) const;

    off_t diffTransfer() const;

    off_t totalTransferred() const;
};



class DAVIX_EXPORT DavixCopy
{
public:
    DavixCopy(Context & c, const RequestParams *params);
    virtual ~DavixCopy();

    void copy(const Uri &source, const Uri &destination, unsigned nstreams, DavixError **error);

    // Callbacks
    typedef void (*PerformanceCallback)(const PerformanceData& perfData, void* data);

    void setPerformanceCallback(PerformanceCallback callback, void *udata);

private:
    struct DavixCopyInternal *d_ptr;

    DavixCopy(const DavixCopy&);
    DavixCopy& operator = (const DavixCopy&);
};

}


#endif // DAVIX_COPY_HPP
