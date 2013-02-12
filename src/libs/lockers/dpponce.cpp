#include "dpponce.hpp"

DppOnce::DppOnce() :
    once_locker(PTHREAD_ONCE_INIT)
{
}


int DppOnce::once(void (*func)()){
    return pthread_once(&once_locker, func);
}
