#ifndef NEONSESSION_HPP
#define NEONSESSION_HPP


#include <davixcontext.hpp>
#include <params/davixrequestparams.hpp>
#include <neonsessionfactory.hpp>

#include <ne_session.h>

namespace Davix{

class NEONSession
{
public:
    NEONSession(Context & c, const Uri & uri, const RequestParams & p, DavixError** err);
    virtual ~NEONSession();


    ne_session* get_ne_sess();

private:
    ne_session* _sess;
    NEONSessionFactory & _f;
};

}

#endif // NEONSESSION_HPP
