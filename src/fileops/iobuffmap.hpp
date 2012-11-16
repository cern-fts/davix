#pragma once
#ifndef DAVIX_IOBUFFMAP_HPP
#define DAVIX_IOBUFFMAP_HPP

#include <davixcontext.hpp>
#include <davixuri.hpp>
#include <params/davixrequestparams.hpp>
#include <libs/lockers/dpplocker.hpp>


namespace Davix {

//
// Internal POSIX like to HTTP RW operation mapper
// provides facilities for caching
//
class IOBuffMap
{
public:
    IOBuffMap(Context & c, const Uri & uri, const RequestParams * params);
    virtual ~IOBuffMap();

    // open the file associated with the davix IOBuffMap
    // do a simple check if the file exist and try to anticipate the next ops
    bool open(int flags, DavixError** err);

    //
    ssize_t read(void* buf, size_t count, DavixError** err);

    //
    ssize_t write(const void* buf, size_t count, DavixError** err);


    //
    off_t lseek(off_t offset, int flags, DavixError** err);

    // position independant read operation,
    // similar to pread except that does not need open() before
    ssize_t getOps(void* buf, size_t count, off_t offset, DavixError** err);

    // position independant write operation,
    // similar to pwrite do not need open() before
    ssize_t putOps(const void* buf, size_t count, off_t offset, DavixError** err);


private:
    Context & _c;
    Uri _uri;
    RequestParams _params;
    size_t _file_size;
    HttpRequest * _read_req;
    DppLock _rwlock;

    // pos
    off_t _pos; // current file offset
    off_t _read_pos; //curent read file offset
    bool _read_endfile;

    // open status
    bool _opened;


    bool checkIsOpen(DavixError** err);
    ssize_t readAheadRequest(void * buffer, size_t size_read, DavixError ** err);
    IOBuffMap(const IOBuffMap &);
    IOBuffMap & operator=(const IOBuffMap &);
};

} // namespace Davix

#endif // DAVIX_IOBUFFMAP_HPP
