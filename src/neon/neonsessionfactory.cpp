#include "neonsessionfactory.hpp"

#include <lockers/dpponce.hpp>
#include <logger/davix_logger_internal.h>
#include <string>
#include <sstream>


#include <cstring>

namespace Davix {

const char* proto_support[] = { "http", "https", NULL };
const unsigned int ports[] = { 80 ,443 , 0};

static DppOnce neon_once;

static void init_neon(){
    ne_sock_init();
}

NEONSessionFactory::NEONSessionFactory() :
    _sess_map(),
    _sess_mut(),
    _session_caching(true)
{
    neon_once.once(&init_neon);
}

NEONSessionFactory::~NEONSessionFactory(){
    DppLocker lock(_sess_mut);
    for(std::multimap<std::string, ne_session*>::iterator it = _sess_map.begin(); it != _sess_map.end(); ++it){
        ne_session_destroy(it->second);
    }
}



int NEONSessionFactory::createNeonSession(const Uri & uri, ne_session** sess, DavixError **err){
    if(uri.getStatus() == StatusCode::OK){
        if(sess != NULL){
            *sess = create_recycled_session(uri.getProtocol(), uri.getHost(),uri.getPort());
            return 0;
        }
    }else{
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::UriParsingError, "impossible to parse " + uri.getString() + " ,not a valid HTTP or Webdav URL");
    }
    return -1;
}

int NEONSessionFactory::storeNeonSession(ne_session* sess, DavixError **err){
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
            DAVIX_DEBUG("cached ne_session found ! taken from cache ");
            se = it->second;
            _sess_map.erase(it);
            return se;
        }

    }
    DAVIX_DEBUG("no cached ne_session, create a new one ");
    return create_session(protocol, host, port);
}

void NEONSessionFactory::internal_release_session_handle(ne_session* sess){
    // clear sensitive data
    // none
    //
    DppLocker lock(_sess_mut);
    std::multimap<std::string, ne_session*>::iterator it;
    std::string sess_key;
    sess_key.append(ne_get_scheme(sess)).append(ne_get_server_hostport(sess));

    DAVIX_DEBUG("add old session to cache %s", sess_key.c_str());

    _sess_map.insert(std::pair<std::string, ne_session*>(sess_key, sess));
}


std::string create_map_keys_from_URL(const std::string & protocol, const std::string &host, unsigned int port){
    std::ostringstream oss;
    if( (strcmp(protocol.c_str(), "http") ==0 && port == 80)
            || ( strcmp(protocol.c_str(), "https") ==0 && port == 443)){
      oss <<  protocol << host;
    }else
        oss <<  protocol << host << ":" << port;
    std::string res = oss.str();
    DAVIX_DEBUG(" creating session keys... %s", res.c_str());
    return res;
}

} // namespace Davix
