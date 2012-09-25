#include "davixuri.h"

namespace Davix {

Uri::Uri(){

}

Uri::Uri(const std::string & uri)
{
    this->uri_string = uri;
    this->d_ptr = NULL;
}

Uri::~Uri(){

}

} // namespace Davix
