#ifndef DAVIX_MUTEX_HPP
#define DAVIX_MUTEX_HPP

#include <pthread.h>

// Simple wrapper for scoped mutex

namespace Davix{

class DavixMutex{

public:
    DavixMutex(pthread_mutex_t& mutex) : _mutex(mutex)
    {
        pthread_mutex_lock(&_mutex);
    }

    ~DavixMutex()
    {
        pthread_mutex_unlock(&_mutex);
    }
    
    void lock() 
    {
        pthread_mutex_lock(&_mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&_mutex);
    }

    pthread_mutex_t* getMutex()
    {
        return &_mutex;
    }

private:
    pthread_mutex_t& _mutex;
};

}

#endif // DAVIX_MUTEX_HPP
