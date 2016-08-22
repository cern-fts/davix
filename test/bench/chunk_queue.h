#ifndef CHUNK_QUEUE
#define CHUNK_QUEUE

#include <pthread.h>
#include <deque>
#include <davix.hpp>

#define DEFAULT_WAIT_TIME 5

const int STARTED = 1;
const int STOPPED = 2;

class ChunkQueue
{
public:

    ChunkQueue();
    ~ChunkQueue();

    struct worktoken
    {
        long length;
        long offset;
        DAVIX_FD* fd;
    };

    void pushOp(long len, long oset, DAVIX_FD* davfd);

    struct worktoken *getOp();

    void StopThreads();
    int GetQueueSize();
    int GetQueueState();
    void SetQueueState(int new_state);


private:

    /// Queue of the pending operations
    std::deque<worktoken*> workqueue;

    pthread_mutex_t workmutex;
    pthread_cond_t popconvar;
    pthread_cond_t pushconvar;

    int state;
};

#endif
