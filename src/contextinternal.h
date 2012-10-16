#ifndef DAVIX_CONTEXTINTERNAL_H
#define DAVIX_CONTEXTINTERNAL_H

#include <tr1/memory>
#include <neon/neonsessionfactory.hpp>

namespace Davix {


/**
   Implementation f the core logic in davix
 */
class ContextInternal
{
public:
    ContextInternal(AbstractSessionFactory * fsess);

    virtual ~ContextInternal(){}

    /**
      implementation of getSessionFactory
    */
    virtual AbstractSessionFactory* getSessionFactory();


     virtual void set_buffer_size(const size_t value);

protected:

    DAVIX_DIR* internal_opendirpp(const char * scope, const std::string & body, const std::string & url  );

    std::auto_ptr<AbstractSessionFactory>  _fsess;
    size_t _s_buff;
    unsigned long _timeout;
};

typedef ContextInternal Core;

} // namespace Davix

#endif // DAVIX_CONTEXTINTERNAL_H
