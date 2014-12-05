#ifndef DAVIX_IGNORED_HPP
#define DAVIX_IGNORED_HPP


// WARNING
// This file contains functions / variables that will be removed from the official API
// This file is maintained for compatbility only
// Please, do not use any function of this header in a new program

#include "../davix.hpp"

std::ostream& operator<< (std::ostream& stream, const Davix::Uri & _u);


#endif // DAVIX_IGNORED_HPP
