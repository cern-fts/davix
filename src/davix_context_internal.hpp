#ifndef DAVIX_CONTEXTINTERNAL_H
#define DAVIX_CONTEXTINTERNAL_H

#include <davixcontext.hpp>
#include <neon/neonsessionfactory.hpp>

namespace Davix {


class ContextExplorer{

static NEONSessionFactory & SessionFactoryFromContext(Context & c);

};


} // namespace Davix

#endif // DAVIX_CONTEXTINTERNAL_H
