#ifndef DPPONCE_HPP
#define DPPONCE_HPP

#include "dpplocker.hpp"

class DppOnce
{
public:
    DppOnce();

    void once( void (*func)(void) );
private:
    int state;
    DppLock l;
};

#endif // DPPONCE_HPP
