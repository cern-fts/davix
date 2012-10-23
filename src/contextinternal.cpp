#include "contextinternal.h"

#include <neon/neonsessionfactory.hpp>
namespace Davix {



// initialization
__attribute__((constructor))
void core_init(){
    if (!g_thread_supported())
      g_thread_init(NULL);
}





ContextInternal::ContextInternal(AbstractSessionFactory* fsess) : _fsess(fsess)
{
    _s_buff = 65536;
    _timeout = 300;
    count_instance =1;
}


AbstractSessionFactory* ContextInternal::getSessionFactory(){
    return _fsess.get();
}

void ContextInternal::set_buffer_size(const size_t value){
    _s_buff = value;
}



HttpRequest* ContextInternal::createRequest(const std::string & uri, DavixError** err){
    return static_cast<HttpRequest*>(getSessionFactory()->create_request(uri, err));
}


} // namespace Davix
