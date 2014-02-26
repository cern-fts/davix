/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) 2013  Adrien Devresse <adrien.devresse@cern.ch>, CERN
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
