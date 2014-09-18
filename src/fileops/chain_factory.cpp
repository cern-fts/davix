#include "chain_factory.hpp"

#include "davmeta.hpp"
#include "httpiovec.hpp"
#include "metalinkops.hpp"
#include "iobuffmap.hpp"

namespace Davix{


ChainFactory::ChainFactory(){}


HttpIOChain& ChainFactory::instanceChain(const CreationFlags & flags, HttpIOChain & c){
    HttpIOChain* elem;
    elem= c.add(new MetalinkOps())->add(new S3MetaOps())->add(new HttpMetaOps());

    // add posix to the chain if needed
    if(flags[CHAIN_POSIX] == true){
        elem = elem->add(new HttpIOBuffer());
    }

    elem->add(new HttpIO())->add(new HttpIOVecOps());
    return c;
}

}
