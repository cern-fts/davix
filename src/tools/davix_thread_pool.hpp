/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Kwong Tat Cheung <kwong.tat.cheung@cern.ch>
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

#ifndef DAVIX_THREAD_POOL_HPP
#define DAVIX_THREAD_POOL_HPP

#include <tools/davix_thread.hpp>
#include <tools/davix_taskqueue.hpp>
#include <davix_internal.hpp>
#include <pthread.h>

namespace Davix{

class DavixThreadPool{
public:
    DavixThreadPool(DavixTaskQueue* tq, const int pool_size);
    ~DavixThreadPool();
    void init();
    void shutdown();
    int getThreadCount();
    bool allThreadsIdle();
private:
    DavixTaskQueue* _tq;
    int threadCount;
    int _pool_size;
    DavixThread** tp;
};

}

#endif // DAVIX_THREAD_POOL_HPP
