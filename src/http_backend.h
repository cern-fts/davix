#ifndef HTTP_BACKEND_H
#define HTTP_BACKEND_H

#ifdef CURL_BACKEND
#include <curl/curlsessionfactory.h>
#elif NEON_BACKEND
#include <neon/neonsessionfactory.h>
#endif

#endif // HTTP_BACKEND_H
