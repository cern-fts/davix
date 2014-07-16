#pragma once
#ifndef DAVIX_NOCOPY_HPP
#define DAVIX_NOCOPY_HPP


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


#endif // DAVIX_NOCOPY_HPP
