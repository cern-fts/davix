#ifndef DAVIX_TASKQUEUE_HPP
#define DAVIX_TASKQUEUE_HPP

#include <davix.hpp>
#include <tools/davix_mutex.hpp>
#include <tools/davix_op.hpp>
#include <pthread.h>

#define DEFAULT_WAIT_TIME 30

namespace Davix{

class DavixTaskQueue{
public:
    DavixTaskQueue();
    int pushOp(DavixOp* op);
    DavixOp* popOp();
    int getSize();
    bool isEmpty();
    void shutdown();
    bool isStopped();
private:
    std::deque<DavixOp*> taskQueue;
    pthread_mutex_t tq_mutex;
    pthread_cond_t popOpConvar;
    pthread_cond_t pushOpConvar;
    bool _shutdown;
};

}

#endif // DAVIX_TASKQUEUE_HPP
