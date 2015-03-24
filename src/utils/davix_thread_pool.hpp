#ifndef DAVIX_THREAD_POOL_HPP
#define DAVIX_THREAD_POOL_HPP

#include <utils/davix_thread.hpp>
#include <utils/davix_taskqueue.hpp>
#include <davix_internal.hpp>
#include <pthread.h>

namespace Davix{

class DavixThreadPool{
public:
    DavixThreadPool(DavixTaskQueue* tq);
    ~DavixThreadPool();
    void init();
    void shutdown();
private:
    DavixThread* tp[DAVIX_DEFAULT_THREADPOOL_SIZE];
    DavixTaskQueue* _tq;
    int threadCount;
};

}

#endif // DAVIX_THREAD_POOL_HPP
