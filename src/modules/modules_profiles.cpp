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
    DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CORE, "Add CA path {} to valid CA path list", env.ca_path);

    std::string proxy = EnvUtils::getEnv("X509_USER_PROXY", std::string());
    if(proxy.size() ==0){
        proxy = fmt::format("/tmp/x509up_u{}", geteuid());
        if(access(proxy.c_str(), R_OK) !=0){
            DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_CORE, "Unable to read proxy file {}", proxy);
            proxy.clear();
        }
    }
    if(proxy.size() > 0){
        DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CORE, "Define {} proxy certificate for use", proxy);
        env.cert_path = env.key_path = proxy;
    }else{
        // No proxy, load simply creds
        env.key_path = EnvUtils::getEnv("X509_USER_KEY", std::string());
        env.cert_path = EnvUtils::getEnv("X509_USER_CERT", std::string());
        DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CORE, "Define to use GRID key {} and GRID cert {} ", env.key_path, env.cert_path);
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
    if(env_grid.key_path.size() > 0){
        X509Credential x509;
        DavixError* tmp_err=NULL;
        if( x509.loadFromFilePEM(env_grid.key_path, env_grid.cert_path, "", &tmp_err) <0){
            DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_CORE, "Impossible to load GRID certificate {} {}: {}",
                      env_grid.key_path,
                      env_grid.cert_path,
                      tmp_err->getErrMsg());
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
