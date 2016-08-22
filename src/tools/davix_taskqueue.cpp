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

#include "davix_taskqueue.hpp"
#include <davix_internal.hpp>
#include <utils/davix_logger_internal.hpp>

namespace Davix{

DavixTaskQueue::DavixTaskQueue(){
    pthread_mutex_init(&tq_mutex, NULL);
    pthread_cond_init(&popOpConvar, NULL);
    pthread_cond_init(&pushOpConvar, NULL);
    _shutdown = false;
    _no_cap = false;
    _queueType = QueueType::FIFO;
}

DavixTaskQueue::DavixTaskQueue(const Tool::OptParams& opts, QueueType::QueueType queueType){
    pthread_mutex_init(&tq_mutex, NULL);
    pthread_cond_init(&popOpConvar, NULL);
    pthread_cond_init(&pushOpConvar, NULL);
    _shutdown = false;
    _no_cap = opts.no_cap;
    _queueType = queueType;
}


int DavixTaskQueue::pushOp(DavixOp* op){

    {
        DavixMutex mutex(tq_mutex);

        // for recursive namespace crawling, we set a hard limit to the number of listing ops are allowed to prevent overflow
        // not the ideal solution, perhaps disk-backing the queue after a certain threshold would be better
        if((op->getOpType() == "LIST") || op->getOpType() == "LISTPP"){
            if(!_no_cap && (taskQueue.size() > MAX_NUM_OF_LISTING_OPS)){
                // queue size balloning out of control, warn and exit
                std::cerr << std::endl << "***Task queue's size has exceeded the maximum allowed, exiting programme!***" << std::endl;
                exit(-1);
            }
        }
        else{
            while(taskQueue.size() >= DAVIX_DEFAULT_TASKQUEUE_SIZE)
                pthread_cond_wait(&pushOpConvar, mutex.getMutex());
        }

        taskQueue.push_back(op);

    }
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "(DavixTaskQueue) {} Op pushed to taskQueue, target is {}, queue size is now: {}", op->getOpType(), op->getTargetUrl(), taskQueue.size());
    pthread_cond_signal(&popOpConvar);
    return 0;
}

DavixOp* DavixTaskQueue::popOp(){
    bool dosig = false;
    DavixOp* op = NULL;
    //struct timespec to;
    {
        DavixMutex mutex(tq_mutex);


        while(taskQueue.empty() && !_shutdown){
            /*
             *        to.tv_sec = time(NULL) + DEFAULT_WAIT_TIME;
             *        to.tv_nsec = 0;
             *        int rc = pthread_cond_timedwait(&popOpConvar, mutex.getMutex(), &to);
             *
             *        if(rc == ETIMEDOUT)
             *        {
             *            std::cerr << std::endl << "(DavixTaskQueue) PopOp() timed out." << std::endl;
             *            return NULL;
        }
        }
        */
            pthread_cond_wait(&popOpConvar, mutex.getMutex());

            if(_shutdown){
                DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "(DavixTaskQueue) Shutting down task queue, queue size is: {}", taskQueue.size());
                return NULL;
            }
        }

        if(taskQueue.size() > 0){
            switch(_queueType){
                case QueueType::FIFO:
                {
                    op = taskQueue.front();
                    taskQueue.pop_front();
                    break;
                }
                case QueueType::LIFO:
                {
                    op = taskQueue.back();
                    taskQueue.pop_back();
                    break;
                }
            }   // end switch
            dosig = true;
        }

    }   // end mutex scope

    if (dosig) {
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "(DavixTaskQueue) {} Op popped from taskQueue, queue size is now: {}", op->getOpType(), taskQueue.size());

        pthread_cond_signal(&pushOpConvar);
        return op;
    }

    return NULL;
}

int DavixTaskQueue::getSize(){
    DavixMutex mutex(tq_mutex);
    return taskQueue.size();
}

bool DavixTaskQueue::isEmpty(){
    DavixMutex mutex(tq_mutex);
    return taskQueue.empty();
}

void DavixTaskQueue::shutdown(){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "(DavixTaskQueue) Commencing shutdown...");
    _shutdown = true;
    pthread_cond_broadcast(&pushOpConvar);
    pthread_cond_broadcast(&popOpConvar);
}

bool DavixTaskQueue::isStopped(){
    return _shutdown;
}

}
