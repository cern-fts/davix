#ifndef DAVIX_NEONSESSIONFACTORY_H
#define DAVIX_NEONSESSIONFACTORY_H

#include <abstractsessionfactory.hpp>
#include <global_def.hpp>
#include <neon/neonrequest.hpp>

namespace Davix {

class NEONSessionFactory : public AbstractSessionFactory
{
    friend class NEONRequest;
public:
    NEONSessionFactory();
    virtual ~NEONSessionFactory();

    /**
      Take the ownership on a request object in order to execute a query
      @param typ : type of the request
      @param url : path of the request
      @return Request object
    */
    virtual Request* take_request(RequestType typ, const std::string & url) ;

    /**
        release the ownership on a request object
     */
    virtual void release_request(Request * req);

    /**
      \ref ssl session check
    */
    virtual void set_ssl_ca_check(bool chk);

    virtual void set_authentification_controller(void * userdata, davix_auth_callback call);

private:
    bool _ca_check;
    void * _user_auth_callback_data;
    davix_auth_callback _call;
    std::multimap<std::string, ne_session*> _sess_map;
    Glib::Mutex _sess_mut;

    void internal_release_session_handle(ne_session* sess);

    ne_session* create_session(const std::string & protocol, const std::string &host, unsigned int port);

    ne_session* create_recycled_session(const std::string & protocol, const std::string &host, unsigned int port);
};

void parse_http_neon_url(const std::string & url, std::string & protocol,
                         std::string & host, std::string & path, unsigned long *port);

std::string create_map_keys_from_URL(const std::string & protocol, const std::string &host, unsigned int port);

} // namespace Davix



#endif // DAVIX_NEONSESSIONFACTORY_H
