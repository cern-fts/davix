#include "httpiochain.hpp"

#include <davix_internal.hpp>

namespace Davix{

HttpIOChain::HttpIOChain() : _next(NULL), _start(this)
{
}

HttpIOChain::~HttpIOChain(){}


HttpIOChain* HttpIOChain::add(HttpIOChain* elem){
    _next.reset(elem);
    if(_next.get() != NULL){
        _next->_start = this->_start;
    }
    return _next.get();
}



// calculate hecksum
void HttpIOChain::checksum(IOChainContext & iocontext, std::string & checksm, const std::string & chk_algo){
    CHAIN_FORWARD(checksum(iocontext, checksm, chk_algo));
}

// calc replica
std::vector<DavFile> &  HttpIOChain::getReplicas(IOChainContext & iocontext, std::vector<DavFile> & vec){
    CHAIN_FORWARD(getReplicas(iocontext, vec));
}

// delete resource
void HttpIOChain::deleteResource(IOChainContext & iocontext){
    CHAIN_FORWARD(deleteResource(iocontext));
}


// make collection
void HttpIOChain::makeCollection(IOChainContext & iocontext){
    CHAIN_FORWARD(makeCollection(iocontext));
}

// move/rename resource
void HttpIOChain::move(IOChainContext & iocontext, const std::string & target_url){
    CHAIN_FORWARD(move(iocontext, target_url));
}

StatInfo & HttpIOChain::statInfo(IOChainContext & iocontext, StatInfo &st_info){
    CHAIN_FORWARD(statInfo(iocontext, st_info));
}

bool HttpIOChain::nextSubItem(IOChainContext &iocontext, std::string &entry_name, StatInfo &info){
    CHAIN_FORWARD(nextSubItem(iocontext, entry_name, info));
}


bool HttpIOChain::open(IOChainContext & iocontext, int flags){
   CHAIN_FORWARD(open(iocontext, flags));
}

 void HttpIOChain::prefetchInfo(IOChainContext & iocontext, off_t offset, dav_size_t size_read, advise_t adv){
     CHAIN_FORWARD(prefetchInfo(iocontext, offset, size_read, adv));
 }

dav_ssize_t HttpIOChain::readFull(IOChainContext & iocontext, std::vector<char> &buffer){
    CHAIN_FORWARD(readFull(iocontext, buffer));
}


dav_ssize_t HttpIOChain::readFull(IOChainContext & iocontext, std::string & str_buffer){
    std::vector<char> buffer;
    dav_ssize_t s = readFull(iocontext, buffer);
    str_buffer.assign(buffer.begin(), buffer.end());
    return s;
}

// read to fd
dav_ssize_t HttpIOChain::readToFd(IOChainContext & iocontext, int fd, dav_size_t size){
    CHAIN_FORWARD(readToFd(iocontext, fd, size));
}


dav_ssize_t HttpIOChain::preadVec(IOChainContext & iocontext, const DavIOVecInput *input_vec, DavIOVecOuput *output_vec, const dav_size_t count_vec){
    CHAIN_FORWARD(preadVec(iocontext, input_vec, output_vec, count_vec));
}

void HttpIOChain::resetIO(IOChainContext & iocontext){
     CHAIN_FORWARD(resetIO(iocontext));
}

dav_ssize_t HttpIOChain::pread(IOChainContext & iocontext, void *buf, dav_size_t count, dav_off_t offset){
    CHAIN_FORWARD(pread(iocontext, buf,count, offset));
}

dav_ssize_t HttpIOChain::read(IOChainContext & iocontext, void *buf, dav_size_t count){
    CHAIN_FORWARD(read(iocontext, buf, count));
}

dav_off_t HttpIOChain::lseek(IOChainContext & iocontext, dav_off_t offset, int flags){
    CHAIN_FORWARD(lseek(iocontext, offset, flags));
}

dav_ssize_t HttpIOChain::writeFromFd(IOChainContext & iocontext, int fd, dav_size_t size){
    CHAIN_FORWARD(writeFromFd(iocontext, fd, size));
}

dav_ssize_t HttpIOChain::writeFromCb(IOChainContext &iocontext, const DataProviderFun &func, dav_size_t size){
    CHAIN_FORWARD(writeFromCb(iocontext, func, size));
}

dav_ssize_t HttpIOChain::write(IOChainContext & iocontext, const void *buf, dav_size_t count){
    CHAIN_FORWARD(write(iocontext, buf, count));
}



} // Davix
