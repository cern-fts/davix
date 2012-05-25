#ifndef GRIDUTILS_H
#define GRIDUTILS_H

#include <httprequest.h>
#include <composition.h>
#include <errno.h>

namespace davix{

void configure_http_request_for_grid_mode(HttpRequest* req, const int grid_level);

}

#endif // GRIDUTILS_H
