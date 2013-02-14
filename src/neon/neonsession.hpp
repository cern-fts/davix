#ifndef NEONSESSION_HPP
#define NEONSESSION_HPP

#include <pthread.h>
#include <davixcontext.hpp>
#include <params/davixrequestparams.hpp>
#include <neon/neonsessionfactory.hpp>

#include <ne_session.h>

namespace Davix{

class NEONSessionFactory;

class NEONSession
{
public:
    NEONSession(Context & c, const Uri & uri, const RequestParams & p, DavixError** err);
    NEONSession(NEONSessionFactory & f, const Uri & uri, const RequestParams & p, DavixError** err);
    virtual ~NEONSession();


    inline ne_session* get_ne_sess(){
        return _sess;
    }

private:
    NEONSessionFactory & _f;
    ne_session* _sess;
    const RequestParams & _params;
    DavixError* _last_error;

    NEONSession(const NEONSession &);
    NEONSession& operator=(const NEONSession &);


    // auth callback mapper
    //
    static void provide_clicert_fn(void *userdata, ne_session *sess,
                                             const ne_ssl_dname *const *dnames,
                                             int dncount);

    static int provide_login_passwd_fn(void *userdata, const char *realm, int attempt,
                                    char *username, char *password);

};


void configureSession(ne_session *_sess, const RequestParams &params, ne_auth_creds lp_callbac, void* lp_userdata,
                      ne_ssl_provide_fn cred_callback,  void* cred_userdata);


}

#endif // NEONSESSION_HPP
