#ifndef DAVIX_TYPES_HPP
#define DAVIX_TYPES_HPP

#include <davix_types.h>

#ifndef __DAVIX_INSIDE__
#error "Only davix.hpp should be included."
#endif





/// @class NonCopyable
/// @brief Simple NonCopyableProtector
class NonCopyable{
protected:
   NonCopyable() {}
    ~NonCopyable() {}
private:  // emphasize the following members are private
   NonCopyable( const NonCopyable& );
   const NonCopyable& operator=( const NonCopyable& );
};



#endif // DAVIX_TYPES_HPP
