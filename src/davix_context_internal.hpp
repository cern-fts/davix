#ifndef DAVIX_CONTEXTINTERNAL_H
#define DAVIX_CONTEXTINTERNAL_H

#include <davixcontext.hpp>
#include <neon/neonsessionfactory.hpp>
#include <utils/davix_logger_internal.hpp>

namespace Davix {

/// @cond HIDDEN_SYMBOLS


struct ContextExplorer{

static NEONSessionFactory & SessionFactoryFromContext(Context & c);

};


///@endcond

} // namespace Davix

#endif // DAVIX_CONTEXTINTERNAL_H
