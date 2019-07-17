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
        perfCallback(NULL), perfCallbackUdata(NULL),
        cancCallback(NULL), cancCallbackUdata(NULL)
    {
    }

    void copy(const Davix::Uri &source, const Davix::Uri &destination,
              unsigned nstreams, Davix::DavixError **error);

    void setPerformanceCallback(DavixCopy::PerformanceCallback callback, void *udata);
    void setCancellationCallback(DavixCopy::CancellationCallback callback, void *udata);

protected:
    Davix::Context &context;
    const Davix::RequestParams *parameters;
    DavixCopy::PerformanceCallback perfCallback;
    void *perfCallbackUdata;

    DavixCopy::CancellationCallback cancCallback;
    void *cancCallbackUdata;

    void monitorPerformanceMarkers(Davix::HttpRequest *request, Davix::DavixError **error);

private:
    DavixCopyInternal(const DavixCopyInternal&);
    DavixCopyInternal& operator = (const DavixCopyInternal&);

    bool shouldCancel();
    bool shouldCancel(Davix::DavixError **error);


};


#endif //DAVIX_COPY_INTERNAL_HPP
