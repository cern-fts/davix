/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
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

    DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CORE, "Enable GRID profile for DAVIX");

    GridEnv env;
    env.ca_path = EnvUtils::getEnv("X509_CERT_DIR", "/etc/grid-security/certificates/");
    DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CORE, "Adding CA path {} to valid CA path list", env.ca_path);

    std::string proxy = EnvUtils::getEnv("X509_USER_PROXY", std::string());
    std::string key = EnvUtils::getEnv("X509_USER_KEY", std::string());
    std::string cert = EnvUtils::getEnv("X509_USER_CERT", std::string());

    std::string standard_location = fmt::format("/tmp/x509up_u{}", geteuid());

    if(!proxy.empty()) {
        DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CORE, "Using X509_USER_PROXY to supply credentials: {}", proxy);
        env.cert_path = env.key_path = proxy;
    }
    else if(access(standard_location.c_str(), R_OK) == 0) {
        DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CORE, "Using standard location for proxy: {}", standard_location);
        env.cert_path = env.key_path = standard_location;
    }
    else if(!cert.empty()) {
        DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CORE, "Using X509_USER_CERT and X509_USER_KEY to supply credentials: {}, {}", cert, key);
        env.cert_path = cert;
        env.key_path = key;
        if(key.empty()) env.key_path = cert;
    }
    else {
        DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_CORE, "Unable to find a proxy or cert/key pair using either X509_USER_* variables or {}", standard_location);
    }

    return env;
}


void awesomeGridHook(RequestParams& p, HttpRequest & req, Uri & u, RequestPreRunHook previous_hook, GridEnv env_grid){

    // initialize environment
    // add grid CA path
    if(env_grid.ca_path.size() >0){
        p.addCertificateAuthorityPath(env_grid.ca_path);
    }
    // if no cert auth configured, configure one
    if(env_grid.cert_path.size() > 0){
        X509Credential x509;
        DavixError* tmp_err=NULL;
        if( x509.loadFromFilePEM(env_grid.key_path, env_grid.cert_path, "", &tmp_err) <0){
            DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_CORE, "Impossible to load GRID certificate {} {}: {}",
                      env_grid.key_path,
                      env_grid.cert_path,
                      tmp_err->getErrMsg());

            if(tmp_err){
                std::cerr << "("<< tmp_err->getErrScope() <<") Error: "<< tmp_err->getErrMsg() << std::endl;
                DavixError::clearError(&tmp_err);
            }
        }else{
            // in current state, GRID profiles ignore all manually defined callbacks
            p.setClientCertCallbackX509(NULL, NULL);
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
    RequestPreRunHook new_hook = std::bind(awesomeGridHook, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, previous_hook, grid_env);
    context.setHook<RequestPreRunHook>(new_hook);
}

}
