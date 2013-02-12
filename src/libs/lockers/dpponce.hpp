#ifndef DPPONCE_HPP
#define DPPONCE_HPP

#include <pthread.h>

class DppOnce
{
public:
    DppOnce();

    int once( void (*func)(void) );
private:
    pthread_once_t once_locker;
};

#endif // DPPONCE_HPP
