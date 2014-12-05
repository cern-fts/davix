#ifndef DAVIX_CONTEXTINTERNAL_H
#define DAVIX_CONTEXTINTERNAL_H

#include <davixcontext.hpp>
#include <neon/neonsessionfactory.hpp>
#include <utils/davix_logger_internal.hpp>

#include <dlfcn.h>


namespace Davix {

/// @cond HIDDEN_SYMBOLS


struct ContextExplorer{

static NEONSessionFactory & SessionFactoryFromContext(Context & c);

};


// libpath handler
struct LibPath{
    LibPath();

    std::string path;

};


const std::string & getLibPath();




///@endcond

} // namespace Davix

#endif // DAVIX_CONTEXTINTERNAL_H
