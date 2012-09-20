#ifndef DAVIX_DAVIXREQUEST_H
#define DAVIX_DAVIXREQUEST_H


namespace Davix {

class Context;

class NGQRequest
{
public:
    NGQRequest(Context* context);

    virtual ~NGQRequest(){};

protected:
    Context* context;
};

} // namespace Davix

#endif // DAVIX_DAVIXREQUEST_H
