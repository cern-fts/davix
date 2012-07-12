#ifndef DAVIX_CPP_HPP
#define DAVIX_CPP_HPP

#include <coreinterface.hpp>

namespace Davix{

/**
    @brief create a davix context : the main entry point of davix

    davix context are :
     - thread safe
     - share automatically http session between operations
     - can be use to contact several servers at the same time
     - can create low level request

*/
CoreInterface* davix_context_create();



}


#endif // DAVIX_CPP_HPP
