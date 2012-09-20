#ifndef DAVIX_HTTPGATE_H
#define DAVIX_HTTPGATE_H

#include <string>
#include <davixgate.h>


namespace Davix {

class Context;
class NGQHttpRequest;

class HttpGate : public Gate
{
public:
    HttpGate(Context* context);


    NGQHttpRequest* createRequest(const std::string & uri);

};

} // namespace Davix

#endif // DAVIX_HTTPGATE_H
