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

#include <tools/davix_thread_pool.hpp>
#include <utils/davix_logger_internal.hpp>

namespace Davix{

DavixThreadPool::DavixThreadPool(DavixTaskQueue* tq, const int pool_size) :
    _tq(tq),
    threadCount(0),
    _pool_size(pool_size)
{
    init();
}

DavixThreadPool::~DavixThreadPool(){
    for(int i=0; i<_pool_size; ++i){
        if(tp[i])
            delete tp[i];
    }
    if(tp) delete tp;
}

void DavixThreadPool::init(){
    tp = new DavixThread*[_pool_size];

    if(tp){
        for(int i=0; i<_pool_size; ++i){
            tp[i] = NULL;
            tp[i] = new DavixThread(_tq, i);
            if(tp[i] != NULL){
                tp[i]->createWorkerThread();
                threadCount++;
            }
            else
                std::cerr << std::endl << "Failed to create worker thread: " << i << std::endl;
        }
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "(DavixThreadPool) Threadpool initiated. Number of worker threads is: {}", threadCount);
    }
    else
        std::cerr << std::endl << "Failed to init threadpool!" << std::endl;
}

void DavixThreadPool::shutdown(){
    int count = 0;
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "(DavixThreadPool) Shutting down threadpool. Number of threads to join: {}", threadCount);
    _tq->shutdown();
    for(int i=0; i<_pool_size; ++i){
        if(tp[i] != NULL){
            tp[i]->shutdown();
            if(pthread_join(tp[i]->getWorkerHandle(), NULL) != 0){
                std::cerr << std::endl << "Failed to join worker thread: " << i << std::endl;
                continue;
            }
            count++;
        }
    }
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "(DavixThreadPool) Threadpool stopped. Number of threads joined: {}/{}", count, threadCount);
}

int DavixThreadPool::getThreadCount(){
    return threadCount;
}

bool DavixThreadPool::allThreadsIdle(){
    for(int i = 0; i < threadCount; ++i){
        if(tp[i]->getWorkerState() == WorkerState::BUSY)
            return false;
    }
    return true;
}

}
