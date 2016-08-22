#include "chunk_queue.h"
#include <sys/time.h>
#include <errno.h>
#include <iostream>

ChunkQueue::ChunkQueue()
{
    pthread_mutex_init(&workmutex, NULL);
    pthread_cond_init(&popconvar, NULL);
    pthread_cond_init(&pushconvar, NULL);
    state = STARTED;
}

ChunkQueue::~ChunkQueue()
{
    for(unsigned int i = 0; i < workqueue.size(); ++i)
    {
        delete(workqueue[i]);
    }
}

void ChunkQueue::pushOp(long len, long oset, DAVIX_FD* davfd)
{
    struct timespec to;
    bool pushed = false;

    pthread_mutex_lock(&workmutex);

    to.tv_sec = time(NULL) + DEFAULT_WAIT_TIME;
    to.tv_nsec = 0;

    while(!pushed)
    {
        if(workqueue.size() < 1000)
        {
            worktoken* tk = new(worktoken);
            tk->length = len;
            tk->offset = oset;
            tk->fd = davfd;
            workqueue.push_back(tk);
            pushed = true;
            break;
        }

        int rc = pthread_cond_timedwait(&pushconvar, &workmutex, &to);
        if(rc == ETIMEDOUT)
        {
            std::cerr << std::endl << "pushOp() timed out." << std::endl;
            break;
        }

    }
    //signal worker, job available
    pthread_mutex_unlock(&workmutex);
    pthread_cond_signal(&popconvar);
}

struct ChunkQueue::worktoken *ChunkQueue::getOp()
{
    struct worktoken* mytk = 0;
    struct timespec to;

    pthread_mutex_lock(&workmutex);

    to.tv_sec = time(NULL) + DEFAULT_WAIT_TIME;
    to.tv_nsec = 0;

    while(!mytk)
    {
        if(workqueue.size() > 0)
        {
            mytk = workqueue.front();
            workqueue.pop_front();
            break;
        }

        int rc = pthread_cond_timedwait(&popconvar, &workmutex, &to);
        if(rc == ETIMEDOUT)
        {
            std::cerr << std::endl << "getOp() timed out." << std::endl;
            break;
        }

    }

    pthread_mutex_unlock(&workmutex);

    // there is now room in the workqueue, signal producer
    pthread_cond_signal(&pushconvar);

    return (mytk);
}

void ChunkQueue::StopThreads()
{
    state = STOPPED;
    pthread_cond_broadcast(&pushconvar);
    pthread_cond_broadcast(&popconvar);
}

int ChunkQueue::GetQueueSize()
{
    return workqueue.size();
}

int ChunkQueue::GetQueueState()
{
    return state;
}

void ChunkQueue::SetQueueState(int new_state)
{
    state = new_state;
}
