#ifndef ABSTRACTSESSIONFACTORY_H
#define ABSTRACTSESSIONFACTORY_H

 
#include "httprequest.hpp"
#include <params/davixrequestparams.hpp>
#include <davixuri.hpp>

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif

namespace Davix{

class HttpRequest;

class AbstractSessionFactory
{
public:

    AbstractSessionFactory(){}

    virtual ~AbstractSessionFactory(){}

};

}

#endif // ABSTRACTSESSIONFACTORY_H
