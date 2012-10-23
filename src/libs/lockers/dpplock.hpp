#ifndef DPPLOCK_HPP
#define DPPLOCK_HPP

#include <pthread.h>

///
/// Simple portable mutex based on pthread
class DppLock
{
public:
    DppLock();
    virtual ~DppLock();

    void lock();

    void unlock();

private:
    pthread_mutex_t  mux;
};

#endif // DPPLOCK_HPP
