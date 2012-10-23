#ifndef DPPLOCKER_HPP
#define DPPLOCKER_HPP


#include "dpplock.hpp"

///
/// Simple portable scoped locker for mutex based on pthread
class DppLocker
{
public:
    DppLocker(DppLock & lock);
    virtual ~DppLocker();

private:
    DppLock& _lock;
};

#endif // DPPLOCKER_HPP
