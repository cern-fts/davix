#include "modules_profiles.hpp"
#include <sstream>

#include <davixcontext.hpp>
#include <utils/davix_logger_internal.hpp>
#include <system_utils/env_utils.hpp>

namespace Davix{


struct GridEnv{
    std::string cert_path;
    std::string key_path;
    std::string ca_path;
};

GridEnv createGridEnv(){

    DAVIX_TRACE("Enable GRID profile for DAVIX");

    GridEnv env;
    env.ca_path = EnvUtils::getEnv("X509_CERT_DIR", "/etc/grid-security/certificates/");
    DAVIX_TRACE("Add CA path %s to valid CA path list", env.ca_path.c_str());

    std::string proxy = EnvUtils::getEnv("X509_USER_PROXY", std::string());
    if(proxy.size() ==0){
        std::ostringstream ss;
        ss << "/tmp/x509up_u" << geteuid();
        proxy = ss.str();
        if(access(proxy.c_str(), R_OK) !=0){
            DAVIX_LOG(DAVIX_LOG_WARNING, "Unable to read proxy file %s", proxy.c_str());
            proxy.clear();
        }
    }
    if(proxy.size() > 0){
        DAVIX_TRACE("Define %s proxy certificate for use", proxy.c_str());
        env.cert_path = env.key_path = proxy;
    }else{
        // No proxy, load simply creds
        env.key_path = EnvUtils::getEnv("X509_USER_KEY", std::string());
        env.cert_path = EnvUtils::getEnv("X509_USER_CERT", std::string());
        DAVIX_TRACE("Define to use GRID key %s and GRID cert %s ", env.key_path.c_str(), env.cert_path.c_str());
    }
    return env;
}


void AwesomeGridHook(RequestParams& p, HttpRequest & req, Uri & u, RequestPreRunHook previous_hook, GridEnv env_grid){

    // initialize environment
    // add grid CA path
    if(env_grid.ca_path.size() >0){
        p.addCertificateAuthorityPath(env_grid.ca_path);
    }
    // if no cert auth configured, configure one
    if(env_grid.key_path.size() > 0){
        X509Credential x509;
        DavixError* tmp_err=NULL;
        if( x509.loadFromFilePEM(env_grid.key_path, env_grid.cert_path, "", &tmp_err) <0){
            DAVIX_LOG(DAVIX_LOG_WARNING, "Impossible to load GRID certificate %s %s: %s",
                      env_grid.key_path.c_str(),
                      env_grid.cert_path.c_str(),
                      tmp_err->getErrMsg().c_str());
        }else{
            p.setClientCertX509(x509);
        }
    }

    if(previous_hook){
        previous_hook(p, req, u);
    }
}


void loadGridProfile(Context & context){
    GridEnv grid_env = createGridEnv();

    RequestPreRunHook previous_hook = context.getHook<RequestPreRunHook>();
    RequestPreRunHook new_hook = std::bind(AwesomeGridHook, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, previous_hook, grid_env);
    context.setHook<RequestPreRunHook>(new_hook);
}

}
