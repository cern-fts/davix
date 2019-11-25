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

#ifndef DAVIX_THREAD_HPP
#define DAVIX_THREAD_HPP

#include <tools/davix_op.hpp>
#include <tools/davix_taskqueue.hpp>
#include <pthread.h>

namespace Davix{

namespace WorkerState{
    enum WorkerState {
        IDLE,
        BUSY,
        STOPPING
    };
}

namespace WorkerEvent{
    enum WorkerEvent {
        WORK,
        STOP
    };
}


class DavixThread{

private:
    int threadId;
    bool _isFree;

    DavixTaskQueue* _tq;
    pthread_t worker;

public:
    /*
     enum WorkerState {
        IDLE,
        BUSY,
        STOPPING
    }state;

    enum WorkerEvent {
        WORK,
        STOP
    }event;
    */
    WorkerState::WorkerState state;
    WorkerEvent::WorkerEvent event;
    DavixThread(DavixTaskQueue* tq, int id);
    ~DavixThread();
    int createWorkerThread();
    static void* startThread(void* args);
    int run();
    pthread_t& getWorkerHandle();
    void shutdown();
    int getThreadId();
    WorkerState::WorkerState getWorkerState();
};

}

#endif // DAVIX_THREAD_HPP
