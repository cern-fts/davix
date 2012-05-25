#ifndef DAVIX_CORE_H
#define DAVIX_CORE_H

#include <utility>
#include <iostream>
#include <exception>

#include "composition.h"
#include "global_def.h"
#include "abstractsessionfactory.h"


namespace Davix {

/**
  composition of the differents HTTP operations
 */
class Core : public Composition
{
public:
    Core(AbstractSessionFactory * fsess);

    static Glib::RefPtr<Core> create(AbstractSessionFactory* fsess);
    /**
         module for high level stat request
     */
    virtual Glib::RefPtr<Stat> getStat();

    /**
      implementation of grid_mode
    */
    virtual void set_grid_mode(const int state);

    virtual bool get_grid_mode();

protected:
    std::auto_ptr<AbstractSessionFactory>  _fsess;
    bool _grid_mode;
};

} // namespace Davix

#endif // DAVIX_CORE_H
