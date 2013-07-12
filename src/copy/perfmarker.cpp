#include <cstdlib>
#include <davix.hpp>

using namespace Davix;


PerformanceMarker::PerformanceMarker():
    index(0), count(0), begin(0), latest(0),
    transferred(0),
    transferAvg(0), transferInstant(0)
{
    // Nothing
}



PerformanceData::PerformanceData():
    begin(0), latest(0),
    markers()
{
    // Nothing
}



PerformanceData::~PerformanceData()
{
}


void PerformanceData::update(const PerformanceMarker& in)
{
    if (markers.size() != in.count)
        markers.resize(in.count);

    if (in.index < 0 || in.index > markers.size())
        return;

    PerformanceMarker& marker = markers[in.index];

    if (marker.begin == 0)
        marker.begin = in.latest;

    // Calculate differences
    time_t absElapsed = in.latest - marker.begin;
    time_t diffElapsed = in.latest - marker.latest;
    off_t diffSize = in.transferred - marker.transferred;

    // Update
    marker.index = in.index;
    marker.count = in.count;
    marker.latest = in.latest;
    marker.transferred = in.transferred;
    if (absElapsed)
        marker.transferAvg = marker.transferred / absElapsed;
    if (diffElapsed)
        marker.transferInstant = diffSize / diffElapsed;

    if (begin == 0 || begin < marker.begin)
        begin = marker.begin;
    if (latest < marker.latest)
        latest = marker.latest;
}



time_t PerformanceData::absElapsed() const
{
    return latest - begin;
}



off_t PerformanceData::avgTransfer(void) const
{
    off_t total = 0;
    for (int i = 0; i < markers.size(); ++i)
        total += markers[i].transferAvg;
    return total;
}



off_t PerformanceData::diffTransfer() const
{
    off_t total = 0;
    for (int i = 0; i < markers.size(); ++i)
        total += markers[i].transferInstant;
    return total;
}



off_t PerformanceData::totalTransferred() const
{
    off_t total = 0;
    for (int i = 0; i < markers.size(); ++i)
        total += markers[i].transferred;
    return total;
}
