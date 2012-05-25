#ifndef DAVIX_COMPOSITION_H
#define DAVIX_COMPOSITION_H

#include <glibmm/error.h>

#include "davix.h"
#include "global_def.h"
#include "gridutils.h"
#include "libdavix_object.h"
#include "abstractsessionfactory.h"
#include "davix_stat.h"




namespace Davix {

class Stat;

class Composition: public Object
{
public:
    Composition();


    /**
      return an accessor to the stat module
     */
    virtual Glib::RefPtr<Stat> getStat()=0;

    /**
      get the current registered session factory
    */
    virtual AbstractSessionFactory* getSessionFactory()=0;

    /**
      enable or disable the tls credential checking
    */


};

} // namespace Davix

#endif // DAVIX_COMPOSITION_H
