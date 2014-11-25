/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) 2013  Adrien Devresse <adrien.devresse@cern.ch>, CERN
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#ifndef NEONSESSION_HPP
#define NEONSESSION_HPP


#include <davix_context_internal.hpp>
#include <params/davixrequestparams.hpp>
#include <neon/neonsessionfactory.hpp>


#include <pthread.h>
#include <ne_session.h>

namespace Davix{

class NEONSessionFactory;

class NEONSession
{
public:
    NEONSession(Context & c, const Uri & uri, const RequestParams & p, DavixError** err);
    virtual ~NEONSession();


    inline ne_session* get_ne_sess(){
        return _sess;
    }

    inline bool getLastError(DavixError** err){
        if(_last_error){
            DavixError::propagateError(err, _last_error);
            _last_error = NULL;
            return true;
        }
        return false;
    }

    inline void disable_session_reuse(){
        DAVIX_LOG(DAVIX_LOG_DEBUG, LOG_SSL, "Disable Session recycling.....");
        _session_recycling = false;
    }

private:
    NEONSessionFactory & _f;
    ne_session* _sess;
    const RequestParams & _params;
    DavixError* _last_error;
    bool _session_recycling;
    Uri _u;

    NEONSession(const NEONSession &);
    NEONSession& operator=(const NEONSession &);


    // auth callback mapper
    //
    static void authNeonCliCertMapper(void *userdata, ne_session *sess,
                                             const ne_ssl_dname *const *dnames,
                                             int dncount);

    static int provide_login_passwd_fn(void *userdata, const char *realm, int attempt,
                                    char *username, char *password);

};


void configureSession(ne_session *_sess, const Uri & uri, const RequestParams &params, ne_auth_creds lp_callbac, void* lp_userdata,
                      ne_ssl_provide_fn cred_callback,  void* cred_userdata);


}

#endif // NEONSESSION_HPP
