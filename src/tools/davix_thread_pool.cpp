#include <tools/davix_thread_pool.hpp>
#include <utils/davix_logger_internal.hpp>

namespace Davix{

DavixThreadPool::DavixThreadPool(DavixTaskQueue* tq) :
    _tq(tq),
    threadCount(0)
{
    init();
}

DavixThreadPool::~DavixThreadPool(){
    for(int i=0; i<DAVIX_DEFAULT_THREADPOOL_SIZE; ++i){
        if(tp[i])
            delete tp[i]; 
    }
}

void DavixThreadPool::init(){
    for(int i=0; i<DAVIX_DEFAULT_THREADPOOL_SIZE; ++i){
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

void DavixThreadPool::shutdown(){
    int count = 0;
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "(DavixThreadPool) Shutting down threadpool. Number of threads to join: {}", threadCount);
    _tq->shutdown();
    for(int i=0; i<DAVIX_DEFAULT_THREADPOOL_SIZE; ++i){
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


}
