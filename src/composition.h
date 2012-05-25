#ifndef DAVIX_COMPOSITION_H
#define DAVIX_COMPOSITION_H

#include <glibmm/error.h>

#include "davix.h"
#include "global_def.h"
#include "libdavix_object.h"
#include "davix_stat.h"




namespace Davix {

class Stat;

class Composition: public Object
{
public:
    Composition();

    /**
      @brief enable or disable grid mode

      grid mode enable or disable a list of functionalities related
      to the grid environment :
      - gsi certificate auto-recognition ( globus )
      - ssl ca verify
      - credential delegation

      @param state : enable or disable grid mode

    */
    virtual void set_grid_mode(const int state)=0;


    virtual bool get_grid_mode()=0;
    /**
      return an accessor to the stat module
     */
    virtual Glib::RefPtr<Stat> getStat()=0;

    /**
      enable or disable the tls credential checking
    */


};

} // namespace Davix

#endif // DAVIX_COMPOSITION_H
