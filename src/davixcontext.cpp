#include <config.h>
#include <davixcontext.hpp>
#include <davixuri.hpp>
#include <neon/neonsessionfactory.hpp>
#include <davix_context_internal.hpp>
#include <boost/scoped_ptr.hpp>



static const std::string _version = DAVIX_VERSION;


namespace Davix{


///  Implementation f the core logic in davix
struct ContextInternal
{
    ContextInternal(NEONSessionFactory * fsess):
        _fsess(fsess),
        _s_buff(65536),
        _timeout(300),
        _context_flags(0){}

    ContextInternal(const ContextInternal & orig):
        _fsess(new NEONSessionFactory()),
        _s_buff(orig._s_buff),
        _timeout(orig._timeout),
        _context_flags(orig._context_flags)
    {}

    virtual ~ContextInternal(){}

    // implementation of getSessionFactory
    inline NEONSessionFactory* getSessionFactory(){
         return _fsess.get();
    }

    void setBufferSize(const dav_size_t value){
      _s_buff = value;
    }

    boost::scoped_ptr<NEONSessionFactory>  _fsess;
    dav_size_t _s_buff;
    unsigned long _timeout;
    bool _context_flags;
};

///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////


Context::Context() :
    _intern(new ContextInternal(new NEONSessionFactory()))
{
}

Context::Context(const Context &c) :
    _intern(new ContextInternal(*(c._intern))){
}

Context & Context::operator=(const Context & c){
    if( this != &c ){
        if( _intern != NULL)
            delete _intern;
        _intern = new ContextInternal(*(c._intern));
    }
    return *this;
}

Context::~Context(){
    delete _intern;
}

Context* Context::clone(){
    return new Context(*this);
}


void Context::setSessionCaching(bool caching){
    _intern->_fsess->setSessionCaching(caching);
}

bool Context::getSessionCaching() const{
    return _intern->_fsess->getSessionCaching();
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

