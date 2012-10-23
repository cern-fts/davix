#include "dpplock.hpp"

DppLock::DppLock()
{
    pthread_mutex_init(&mux,NULL);
}

DppLock::~DppLock(){
    pthread_mutex_destroy(&mux);
}

void DppLock::lock(){
    pthread_mutex_lock(&mux);
}

void DppLock::unlock(){
    pthread_mutex_unlock(&mux);
}
