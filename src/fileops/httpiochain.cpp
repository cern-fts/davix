#include "httpiochain.hpp"

#include <davix_internal.hpp>

namespace Davix{

HttpIOChain::HttpIOChain(Context &c) : _context(c), _uri(NULL), _params(NULL)
{
}

HttpIOChain::~HttpIOChain(){}

void HttpIOChain::configure(const Uri &uri, const RequestParams *params){
    if(next.get() != NULL){
        configure(uri, params);
    }
    _uri = &uri;
    _params = params;
}

HttpIOChain* HttpIOChain::add(HttpIOChain* elem){
    next.reset(elem);
    return next.get();
}



// calculate hecksum
void HttpIOChain::checksum(std::string & checksm, const std::string & chk_algo){
    CHAIN_FORWARD(checksum(checksm, chk_algo));
}

// calc replica
void HttpIOChain::getReplicas(std::vector<DavFile> & vec){
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


void HttpIOChain::statInfo(StatInfo &st_info){
    CHAIN_FORWARD(statInfo(st_info));
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


dav_ssize_t HttpIOChain::readPartialBufferVec(const DavIOVecInput *input_vec, DavIOVecOuput *output_vec, const dav_size_t count_vec){
    CHAIN_FORWARD(readPartialBufferVec(input_vec, output_vec, count_vec));
}

void HttpIOChain::resetIO(){
     CHAIN_FORWARD(resetIO());
}

dav_ssize_t HttpIOChain::read(void *buf, dav_size_t count, dav_off_t offset){
    CHAIN_FORWARD(read(buf,count, offset));
}

dav_ssize_t HttpIOChain::read(void *buf, dav_size_t count){
    CHAIN_FORWARD(read(buf, count));
}

dav_ssize_t HttpIOChain::writeFromFd(int fd, dav_size_t size){
    CHAIN_FORWARD(writeFromFd(fd, size));
}



} // Davix
