#include "davix_rw.h"
#include <posix/davposix.hpp>



namespace Davix{



ssize_t DavPosix::read(DAVIX_FD* fd, void* buf, size_t count){
    return -1;
}


ssize_t write(DAVIX_FD* fd, const void* buf, size_t count){
    return -1;
}



off_t lseek(DAVIX_FD* fd, off_t offset, int flags){
    return -1;
}


int close(DAVIX_FD* fd){
    return 0;
}



}
