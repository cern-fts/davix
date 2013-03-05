#include <config.h>
#include <davixcontext.hpp>
#include <davixuri.hpp>
#include <neon/neonsessionfactory.hpp>
#include <davix_context_internal.hpp>


/**
 * @cond HIDDEN_SYMBOLS
 */

static const std::string _version = DAVIX_VERSION;

/// @internal

namespace Davix{


///  Implementation f the core logic in davix
struct ContextInternal
{
public:
    ContextInternal(NEONSessionFactory * fsess);

    virtual ~ContextInternal(){}

    /**
      implementation of getSessionFactory
    */
    virtual NEONSessionFactory* getSessionFactory();


     virtual void setBufferSize(const size_t value);


    static ContextInternal* takeRef(ContextInternal* me){
        DppLocker(me->l_counter);
        (me->count_instance)++;
        return me;
    }

    static void releaseRef(ContextInternal* me){
        {
            DppLocker(me->l_counter);
            (me->count_instance)--;
        }
        if(me->count_instance <=0)
            delete me;
    }



    DAVIX_DIR* internal_opendirpp(const char * scope, const std::string & body, const std::string & url  );

    std::auto_ptr<NEONSessionFactory>  _fsess;
    size_t _s_buff;
    unsigned long _timeout;
    DppLock l_counter;
    volatile int count_instance;
};

///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////


ContextInternal::ContextInternal(NEONSessionFactory* fsess) :
    _fsess(fsess),
    _s_buff(65536),
    _timeout(300),
    l_counter(),
    count_instance(1)
{
}


NEONSessionFactory* ContextInternal::getSessionFactory(){
    return _fsess.get();
}

void ContextInternal::setBufferSize(const size_t value){
    _s_buff = value;
}



///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////


Context::Context() :
    _intern(new ContextInternal(new NEONSessionFactory()))
{
}

Context::Context(const Context &c) :
    _intern(ContextInternal::takeRef(c._intern)){
}

Context & Context::operator=(const Context & c){
    if(this->_intern != NULL)
        ContextInternal::releaseRef(this->_intern);
    this->_intern = ContextInternal::takeRef(c._intern);
    return *this;
}

Context::~Context(){
    ContextInternal::releaseRef(_intern);

}

Context* Context::clone(){

    return new Context(*this);
}


HttpRequest* Context::createRequest(const std::string & url, DavixError** err){
    return new HttpRequest(*this, Uri(url), err);
}


HttpRequest* Context::createRequest(const Uri &uri, DavixError **err){
    return new HttpRequest(*this, uri, err);
}


NEONSessionFactory & ContextExplorer::SessionFactoryFromContext(Context & c){
    return *static_cast<NEONSessionFactory*>(c._intern->getSessionFactory());
}


const std::string & version(){
    return _version;
}


} // End Davix



/**
 * @endcond
 **/

DAVIX_C_DECL_BEGIN

/*
int davix_auth_set_pkcs12_cli_cert(davix_auth_t token, const char* filename_pkcs, const char* passwd, davix_error_t* err){
    Davix::NEONSession* req = (Davix::NEONSession*)(token);
    Davix::DavixError* tmp_err=NULL;

    if( req->do_pkcs12_cert_authentification(filename_pkcs, passwd, &tmp_err) <0){
        Davix::DavixError::propagateError((Davix::DavixError**) err, tmp_err);
        return -1;
    }

    return 0;
}

int davix_auth_set_login_passwd(davix_auth_t token, const char* login, const char* passwd, davix_error_t* err){
    Davix::NEONSession* req = (Davix::NEONSession*)(token);
    Davix::DavixError* tmp_err=NULL;

    if(req->do_login_passwd_authentification(login, passwd, &tmp_err) <0){
        Davix::DavixError::propagateError((Davix::DavixError**) err, tmp_err);
        return -1;
    }
    return 0;
}*/

davix_sess_t davix_context_new(davix_error_t* err){
    Davix::Context* comp = new Davix::Context();
    return (davix_sess_t) comp;

}

davix_sess_t davix_context_copy(davix_sess_t sess){
    return (davix_sess_t) new Davix::Context(*((Davix::Context*) sess));
}



void davix_context_free(davix_sess_t sess){
    if(sess != NULL)
        delete ((Davix::Context*)(sess));
}

DAVIX_C_DECL_END

/// @endinternal
