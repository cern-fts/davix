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

#include <cstdlib>
#include <davix.hpp>
#include <utils/davix_logger_internal.hpp>

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
    if(in.count > 8192) {
        DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_GRID, "Received unreasonably high number of stripes, something is wrong: {}", in.count);
        return;
    }

    if (markers.size() != in.count)
        markers.resize(in.count);

    if (in.index < 0 || in.index >= markers.size())
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
    for (size_t i = 0; i < markers.size(); ++i)
        total += markers[i].transferInstant;
    return total;
}



off_t PerformanceData::totalTransferred() const
{
    off_t total = 0;
    for (size_t i = 0; i < markers.size(); ++i)
        total += markers[i].transferred;
    return total;
}
