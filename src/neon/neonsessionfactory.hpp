#ifndef DAVIX_NEONSESSIONFACTORY_H
#define DAVIX_NEONSESSIONFACTORY_H

#include <map>
#include <davixuri.hpp>
 
#include <neon/neonrequest.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <adevpp/containers/cache.hpp>

namespace Davix {

class HttpRequest;

class NEONSessionFactory
{
    friend class NEONRequest;
public:
    NEONSessionFactory();
    virtual ~NEONSessionFactory();

    /**
      Create a session object or create a recycled  one ( session reuse )
    */
    int createNeonSession(const Uri & uri, ne_session** sess, DavixError** err);

    /**
      store a Neon session object for session reuse purpose
    */
    int storeNeonSession(ne_session *sess, DavixError** err);

    inline void setSessionCaching(bool caching){
        _session_caching = caching;
    }

    inline bool getSessionCaching() const {
        return _session_caching;
    }

private:
    // session pool
    std::multimap<std::string, ne_session*> _sess_map;
    boost::mutex _sess_mut;
    bool _session_caching;

    void internal_release_session_handle(ne_session* sess);

    ne_session* create_session(const std::string & protocol, const std::string &host, unsigned int port);

    ne_session* create_recycled_session(const std::string & protocol, const std::string &host, unsigned int port);


    // redirection pool
    Adevpp::Cache<std::string, Uri> _redirCache;
public:

    void addRedirection( const std::string & method, const Uri & origin, boost::shared_ptr<Uri> dest);

    boost::shared_ptr<Uri> redirectionResolve(const std::string & method, const Uri & origin);

    void redirectionClean(const std::string & method, const Uri & origin);

};

void parse_http_neon_url(const std::string & url, std::string & protocol,
                         std::string & host, std::string & path, unsigned long *port);

std::string create_map_keys_from_URL(const std::string & protocol, const std::string &host, unsigned int port);

} // namespace Davix



#endif // DAVIX_NEONSESSIONFACTORY_H
