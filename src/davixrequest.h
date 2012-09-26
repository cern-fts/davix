#ifndef DAVIX_DAVIXREQUEST_H
#define DAVIX_DAVIXREQUEST_H


namespace Davix {

class Context;
class StatusRequest;

class NGQRequest
{
public:
    NGQRequest(Context* context);

    virtual ~NGQRequest(){};

    virtual StatusRequest* executeRequest()=0;

protected:
    Context* context;
};

} // namespace Davix

#endif // DAVIX_DAVIXREQUEST_H
