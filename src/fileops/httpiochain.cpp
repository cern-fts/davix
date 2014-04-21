#include "httpiochain.hpp"

#include <davix_internal.hpp>

namespace Davix{

HttpIOChain::HttpIOChain() : _next(NULL), _params(NULL), _start(this)
{
}

HttpIOChain::~HttpIOChain(){}

void HttpIOChain::configure(Context& c, const Uri &uri, const RequestParams *params){
    _params.reset(new IOChainParams(c, uri, params));
}

HttpIOChain* HttpIOChain::add(HttpIOChain* elem){
    _next.reset(elem);
    if(_next.get() != NULL){
        _next->_start = this->_start;
    }
    return _next.get();
}



// calculate hecksum
void HttpIOChain::checksum(std::string & checksm, const std::string & chk_algo){
    CHAIN_FORWARD(checksum(checksm, chk_algo));
}

// calc replica
std::vector<DavFile> &  HttpIOChain::getReplicas(std::vector<DavFile> & vec){
    CHAIN_FORWARD(getReplicas(vec));
}

// delete resource
void HttpIOChain::deleteResource(){
    CHAIN_FORWARD(deleteResource());
}


// make collection
void HttpIOChain::makeCollection(){
    CHAIN_FORWARD(makeCollection());
}


StatInfo & HttpIOChain::statInfo(StatInfo &st_info){
    CHAIN_FORWARD(statInfo(st_info));
}


bool HttpIOChain::open(int flags){
   CHAIN_FORWARD(open(flags));
}

 void HttpIOChain::prefetchInfo(off_t offset, dav_size_t size_read, advise_t adv){
     CHAIN_FORWARD(prefetchInfo(offset, size_read, adv));
 }

dav_ssize_t HttpIOChain::readFull(std::vector<char> &buffer){
    CHAIN_FORWARD(readFull(buffer));
}


dav_ssize_t HttpIOChain::readFull(std::string & str_buffer){
    std::vector<char> buffer;
    dav_ssize_t s = readFull(buffer);
    str_buffer.assign(buffer.begin(), buffer.end());
    return s;
}

// read to fd
dav_ssize_t HttpIOChain::readToFd(int fd, dav_size_t size){
    CHAIN_FORWARD(readToFd(fd, size));
}


dav_ssize_t HttpIOChain::preadVec(const DavIOVecInput *input_vec, DavIOVecOuput *output_vec, const dav_size_t count_vec){
    CHAIN_FORWARD(preadVec(input_vec, output_vec, count_vec));
}

void HttpIOChain::resetIO(){
     CHAIN_FORWARD(resetIO());
}

dav_ssize_t HttpIOChain::pread(void *buf, dav_size_t count, dav_off_t offset){
    CHAIN_FORWARD(pread(buf,count, offset));
}

dav_ssize_t HttpIOChain::read(void *buf, dav_size_t count){
    CHAIN_FORWARD(read(buf, count));
}

dav_off_t HttpIOChain::lseek(dav_off_t offset, int flags){
    CHAIN_FORWARD(lseek(offset, flags));
}

dav_ssize_t HttpIOChain::writeFromFd(int fd, dav_size_t size){
    CHAIN_FORWARD(writeFromFd(fd, size));
}

dav_ssize_t HttpIOChain::write(const void *buf, dav_size_t count){
    CHAIN_FORWARD(write(buf, count));
}



} // Davix
