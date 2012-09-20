#ifndef DAVIX_CONTEXTINTERNAL_H
#define DAVIX_CONTEXTINTERNAL_H

#include <tr1/memory>
#include <abstractsessionfactory.hpp>

namespace Davix {

class ContextInternal
{
public:
    ContextInternal();

    AbstractSessionFactory* getSessionFactory();

protected:

    std::auto_ptr<AbstractSessionFactory>  _fsess;
};

} // namespace Davix

#endif // DAVIX_CONTEXTINTERNAL_H
