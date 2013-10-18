#ifndef DAVIX_CONTEXTINTERNAL_H
#define DAVIX_CONTEXTINTERNAL_H

#include <davixcontext.hpp>
#include <neon/neonsessionfactory.hpp>
#include <libs/logger/davix_logger_internal.h>

namespace Davix {

/// @cond HIDDEN_SYMBOLS


struct ContextExplorer{

static NEONSessionFactory & SessionFactoryFromContext(Context & c);

};


///@endcond

} // namespace Davix

#endif // DAVIX_CONTEXTINTERNAL_H
