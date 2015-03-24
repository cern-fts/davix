#ifndef DAVIX_THREAD_HPP
#define DAVIX_THREAD_HPP

#include <utils/davix_op.hpp>
#include <utils/davix_taskqueue.hpp>
#include <pthread.h>

namespace Davix{

class DavixThread{

private:
    int threadId;
    bool _isFree;

    enum WorkerState {
        IDLE,
        BUSY,
        STOPPING
    }state;
    
    enum WorkerEvent {
        WORK,
        STOP
    }event;

    DavixTaskQueue* _tq;
    pthread_t worker;
    pthread_cond_t eventConvar;

public:
    DavixThread(DavixTaskQueue* tq, int id);
    ~DavixThread();
    int createWorkerThread();
    static void* startThread(void* args);
    int run();
    pthread_t& getWorkerHandle();
    void shutdown();
    int getThreadId();
    WorkerState getWorkerState();
    
};

}

#endif // DAVIX_THREAD_HPP
