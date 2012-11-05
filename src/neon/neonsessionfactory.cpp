#include "neonsessionfactory.hpp"

#include <string>
#include <sstream>


#include <cstring>

namespace Davix {

const char* proto_support[] = { "http", "https", NULL };
const unsigned int ports[] = { 80 ,443 , 0};

static GOnce neon_once = G_ONCE_INIT;

static gpointer init_neon(gpointer useless){
    ne_sock_init();
    return NULL;
}

NEONSessionFactory::NEONSessionFactory()
{
    g_once (&neon_once, init_neon, NULL);

    if(davix_get_log_filter() & G_LOG_LEVEL_DEBUG){
        davix_log_debug("Enable Debug mode in NEON ...");
        ne_debug_init(stderr, NE_DBG_HTTP | NE_DBG_HTTPAUTH | NE_DBG_HTTPPLAIN | NE_DBG_HTTPBODY);
    }
}

NEONSessionFactory::~NEONSessionFactory(){
    DppLocker lock(_sess_mut);
    for(std::multimap<std::string, ne_session*>::iterator it = _sess_map.begin(); it != _sess_map.end(); ++it){
        ne_session_destroy(it->second);
    }
}

HttpRequest* NEONSessionFactory::create_request(const std::string &url, DavixError** err){;
    Uri uri(url);
    return create_request(uri, err);
}

HttpRequest* NEONSessionFactory::create_request(const Uri &uri, DavixError **err){
    HttpRequest* req = NULL;

    if(uri.getStatus() == StatusCode::OK){
        ne_session* sess = create_recycled_session(uri.getProtocol(), uri.getHost(), uri.getPort());
        req = new HttpRequest(new NEONRequest(this, sess, uri.getPath() ));
    }else{
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::UriParsingError, "impossible to parse " + uri.getString() + " ,not a valid HTTP or Webdav URL");
    }
    return req;
}

int NEONSessionFactory::createNeonSession(const Uri & uri, ne_session** sess){
    if(sess != NULL){
        *sess = create_recycled_session(uri.getProtocol(), uri.getHost(),uri.getPort());
        return 0;
    }
    return -1;
}

int NEONSessionFactory::storeNeonSession(const Uri & uri, ne_session* sess){
    internal_release_session_handle(sess);
    return 0;
}




ne_session* NEONSessionFactory::create_session(const std::string & protocol, const std::string &host, unsigned int port){
    ne_session *se;
    se = ne_session_create(protocol.c_str(), host.c_str(), (int) port);
    //ne_ssl_trust_default_ca(se); not stable in neon on epel 5
    return se;
}

ne_session* NEONSessionFactory::create_recycled_session(const std::string &protocol, const std::string &host, unsigned int port){

    ne_session* se= NULL;
    {
        DppLocker lock(_sess_mut);
        std::multimap<std::string, ne_session*>::iterator it;
        if( (it = _sess_map.find(create_map_keys_from_URL(protocol, host, port))) != _sess_map.end()){
            davix_log_debug("cached ne_session found ! taken from cache ");
            se = it->second;
            _sess_map.erase(it);
            return se;
        }

    }
    davix_log_debug("no cached ne_session, create a new one ");
    return create_session(protocol, host, port);
}

void NEONSessionFactory::internal_release_session_handle(ne_session* sess){
    // clear sensitive data
    // none
    //
    DppLocker lock(_sess_mut);
    std::multimap<std::string, ne_session*>::iterator it;
    const std::string protocol(ne_get_scheme(sess));
    const std::string hostport(ne_get_server_hostport(sess));
    davix_log_debug("add old session to cache %s%s", protocol.c_str(), hostport.c_str());

    _sess_map.insert(std::pair<std::string, ne_session*>(protocol + hostport, sess));
}


std::string create_map_keys_from_URL(const std::string & protocol, const std::string &host, unsigned int port){
    std::ostringstream oss;
    if( (strcmp(protocol.c_str(), "http") ==0 && port == 80)
            || ( strcmp(protocol.c_str(), "https") ==0 && port == 443)){
      oss <<  protocol << host;
    }else
        oss <<  protocol << host << ":" << port;
    std::string res = oss.str();
    davix_log_debug(" creating session keys... %s", res.c_str());
    return res;
}

} // namespace Davix
