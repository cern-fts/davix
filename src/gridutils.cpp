#include "gridutils.h"

namespace davix{

void configure_http_request_for_grid_mode(HttpRequest* req, const int grid_level){
    if(grid_level >= 2) // if grid_level = advanced, disable the ssl checking : worker node mode
        req->disable_ssl_ca_check();
}

}

