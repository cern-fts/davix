#ifndef DAVIX_NEONSESSIONFACTORY_H
#define DAVIX_NEONSESSIONFACTORY_H

#include <map>
#include <abstractsessionfactory.hpp>
#include <davixuri.hpp>
#include <global_def.hpp>
#include <neon/neonrequest.hpp>
#include <libs/lockers/dpplocker.hpp>

namespace Davix {

class HttpRequest;

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
    virtual HttpRequest* create_request(const std::string & url, DavixError** err) ;
    virtual HttpRequest* create_request(const Uri & uri, DavixError** err) ;

    /**
      Create a session object or create a recycled  one ( session reuse )
    */
    int createNeonSession(const Uri & uri, ne_session** sess, DavixError** err);

    /**
      store a Neon session object for session reuse purpose
    */
    int storeNeonSession(ne_session *sess, DavixError** err);

private:
    std::multimap<std::string, ne_session*> _sess_map;
    DppLock _sess_mut;

    void internal_release_session_handle(ne_session* sess);



    ne_session* create_session(const std::string & protocol, const std::string &host, unsigned int port);

    ne_session* create_recycled_session(const std::string & protocol, const std::string &host, unsigned int port);
};

void parse_http_neon_url(const std::string & url, std::string & protocol,
                         std::string & host, std::string & path, unsigned long *port);

std::string create_map_keys_from_URL(const std::string & protocol, const std::string &host, unsigned int port);

} // namespace Davix



#endif // DAVIX_NEONSESSIONFACTORY_H
