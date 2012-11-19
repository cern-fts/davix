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


    static ContextInternal* takeRef(ContextInternal* me){
        g_atomic_int_inc(&(me->count_instance));
        return me;
    }

    static void releaseRef(ContextInternal* me){
        if(g_atomic_int_dec_and_test(&(me->count_instance)))
            delete me;
    }


    HttpRequest* createRequest(const std::string & uri, DavixError** err);

    HttpRequest* createRequest(const Uri & uri, DavixError** err);

protected:

    DAVIX_DIR* internal_opendirpp(const char * scope, const std::string & body, const std::string & url  );

    std::auto_ptr<AbstractSessionFactory>  _fsess;
    size_t _s_buff;
    unsigned long _timeout;
    gint count_instance;
};

} // namespace Davix

#endif // DAVIX_CONTEXTINTERNAL_H
