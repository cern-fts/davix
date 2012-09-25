#include "contextconfig.h"
#include <libs/time_utils.h>

namespace Davix{

ContextConfig::ContextConfig()
{
    timespec_clear(&connexion_timeout);
    timespec_clear(&ops_timeout);
    connexion_timeout.tv_sec = DAVIX_DEFAULT_CONN_TIMEOUT;
    ops_timeout.tv_sec = DAVIX_DEFAULT_OPS_TIMEOUT;
    call =NULL;
    userdata = NULL;
}

ContextConfig::ContextConfig(const ContextConfig &conf){
    timespec_copy(&connexion_timeout, &(conf.connexion_timeout));
    timespec_copy(&ops_timeout, &(conf.ops_timeout));
    call =conf.call;
    userdata = conf.userdata;
}

ContextConfig::~ContextConfig(){

}

void ContextConfig::setAuthCallback(void *userdata, davix_auth_callback call){
    this->userdata = userdata;
    this->call = call;
}


void ContextConfig::setConnexionTimeout(struct timespec *conn_timeout1){
    timespec_copy(&(this->connexion_timeout),conn_timeout1);
}

void ContextConfig::setOperationTimeout(struct timespec *ops_timeout1){
    timespec_copy(&(this->ops_timeout), ops_timeout1);
}

const struct timespec* ContextConfig::getConnexionTimeout(){
    return &connexion_timeout;
}

const struct timespec* ContextConfig::getOperationTimeout(){
    return &ops_timeout;
}

}
