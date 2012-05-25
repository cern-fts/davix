#ifndef DAVIX_CORE_H
#define DAVIX_CORE_H

#include <utility>
#include <iostream>
#include <exception>

#include "composition.h"
#include "global_def.h"



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
      implementation of getSessionFactory
    */
    virtual AbstractSessionFactory* getSessionFactory();

protected:
    std::auto_ptr<AbstractSessionFactory>  _fsess;
};

} // namespace Davix

#endif // DAVIX_CORE_H
