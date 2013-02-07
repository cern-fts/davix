#ifndef GLOBAL_DEF_H
#define GLOBAL_DEF_H

#include <config.h>

#include <iostream>
#include <memory>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <logger/davix_logger.h>

#include <davix.h>

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif


namespace Davix{


#define DAVIX_BUFFER_SIZE 2048

/// default connexion timeout for HTTP/Dav connexion ( 0 : def value of the library )
#define DAVIX_DEFAULT_CONN_TIMEOUT 180
/// default timeout on operations for HTTP/Webdav ( 0 : def value of the library )
#define DAVIX_DEFAULT_OPS_TIMEOUT 0


}

#endif // GLOBAL_DEF_H
