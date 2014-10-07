#include <davix_internal.hpp>
#include <auth/davixauth.hpp>
namespace Davix{

std::vector<std::string> v;

SessionInfo::SessionInfo() : data(NULL){

}

std::vector<std::string> & SessionInfo::getReadableDN() const{
    // TODO: implement proper DN mapping
    return v;
}

}
