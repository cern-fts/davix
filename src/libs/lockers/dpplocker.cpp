#include "dpplocker.hpp"

DppLocker::DppLocker(DppLock & lock) : _lock(lock)
{
    lock.lock();
}

DppLocker::~DppLocker(){
    _lock.unlock();
}
