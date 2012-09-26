#ifndef DAVIX_CONTEXTINTERNAL_H
#define DAVIX_CONTEXTINTERNAL_H

#include <tr1/memory>
#include <neon/neonsessionfactory.hpp>

namespace Davix {

class ContextInternal
{
public:
    ContextInternal();

    NEONSessionFactory* getSessionFactory();

protected:

    std::auto_ptr<NEONSessionFactory>  _fsess;
};

} // namespace Davix

#endif // DAVIX_CONTEXTINTERNAL_H
