#include "dpponce.hpp"

DppOnce::DppOnce() :
    state(0),
    l()
{
}


void DppOnce::once(void (*func)()){
    if(!state){
        DppLocker locker(l);
        if(!state){
            state = 1;
            func();
        }
    }
}
