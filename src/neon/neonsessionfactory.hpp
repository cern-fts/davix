#ifndef DAVIX_NEONSESSIONFACTORY_H
#define DAVIX_NEONSESSIONFACTORY_H

#include <abstractsessionfactory.hpp>
#include <davixuri.hpp>
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
    virtual Request* create_request(const std::string & url, DavixError** err) ;

    /**
      Create a session object or create a recycled  one ( session reuse )
    */
    int createNeonSession(const Uri & uri, ne_session** sess);

    /**
      store a Neon session object for session reuse purpose
    */
    int storeNeonSession(const Uri & uri, ne_session *sess);

    /**
        release the ownership on a request object
     */
    virtual void delete_request(Request * req);


private:
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
