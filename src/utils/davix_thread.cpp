#include "davix_thread.hpp"
#include <utils/davix_logger_internal.hpp>

namespace Davix{

DavixThread::DavixThread(DavixTaskQueue* tq, int id) :
    threadId(id),
    state(IDLE),
    event(WORK),
    _tq(tq),
    worker()
{
}

DavixThread::~DavixThread(){}

int DavixThread::createWorkerThread(){
    if(pthread_create(&worker, NULL, startThread, (void*)this) != 0){
        return -1;
    }
    return 0;
}

void* DavixThread::startThread(void* args){
    DavixThread* worker = (DavixThread*)args;
    worker->run();
    return 0;
}

int DavixThread::run(){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "(DavixThread) Worker thread: {} started", threadId);

    while(true){
        switch(event){
            case WORK:
            {
                DavixOp* op = NULL;
                op = _tq->popOp();

                if(op != NULL){
                    state = BUSY;

                    int ret = op->executeOp();
                    if(ret < 0)
                        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "(DavixThread) Worker thread: {} failed to execute op on {}.", 
                                    threadId, op->getTargetUrl());
                    
                    delete op;
                    state = IDLE;
                }
                break;
            }
            case STOP:
            {
                return 0;
                break;
            }
        }

        // if task queue is not running, nothing to do, exit
        if(_tq->isStopped())
            return 0;
    }
    return 0;
}

pthread_t& DavixThread::getWorkerHandle(){
    return worker;
}

int DavixThread::getThreadId(){
    return threadId;
}

void DavixThread::shutdown(){
    state = STOPPING;
    event = STOP;
}

DavixThread::WorkerState DavixThread::getWorkerState(){
    return state;
}

} // namespace Davix
