#include "posixgate.h"

namespace Davix {

PosixGate::PosixGate(Context* context) : Gate(context)
{
}


/// opendir operation
DAVIX_DIR* PosixGate::opendir(const std::string &url){

    //
    return NULL;
}


///
struct dirent* PosixGate::readdir(DAVIX_DIR* p){
    return NULL;
}

///
void PosixGate::closedir(DAVIX_DIR* p){

}

///
DAVIX_DIR* PosixGate::opendirpp(const std::string & url){
    return NULL;
}



///
struct dirent* PosixGate::readdirpp(DAVIX_DIR*, struct stat * st ){
    return NULL;
}


///
void PosixGate::closedirpp(DAVIX_DIR* ){

}

///
void PosixGate::mkdir(const std::string & url, mode_t right){

}




} // namespace Davix
