#include "neonsession.hpp"
#include <davix_context_internal.hpp>

namespace Davix{

NEONSession::NEONSession(Context & c, const Uri & uri, const RequestParams & p, DavixError** err) :
    _f(ContextExplorer::SessionFactoryFromContext(Context &c)),
    _sess(NULL)
{
        _f.createNeonSession(uri, &_sess, err);

}

NEONSession::~NEONSession(){
    if(_sess)
        _f.storeNeonSession(_sess, NULL);
}


ne_session* NEONSession::get_ne_sess(){
    return _sess;
}

}
