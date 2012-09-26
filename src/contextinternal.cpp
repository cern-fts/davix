#include "contextinternal.h"

#include <neon/neonsessionfactory.hpp>
namespace Davix {

ContextInternal::ContextInternal() : _fsess(new NEONSessionFactory())
{
}

NEONSessionFactory* ContextInternal::getSessionFactory(){
    return _fsess.get();
}

} // namespace Davix
