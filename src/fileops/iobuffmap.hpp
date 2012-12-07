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
/*
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

    // commit any pending operation on the file descriptor
    int commit(DavixError** err);

private:
    Context & _c;
    Uri _uri;
    RequestParams _params;
    HttpRequest * _read_req;
    DppLock _rwlock;

    // pos
    off_t _pos; // current file offset
    off_t _read_pos; //curent read file offset
    bool _read_endfile;

    // open info
    bool _opened;
    bool _file_exist;
    size_t _file_size;


    // cache params
    FILE* _cache_stream;


    bool checkIsOpen(DavixError** err);
    ssize_t readAheadRequest(void * buffer, size_t size_read, DavixError ** err);
    IOBuffMap(const IOBuffMap &);
    IOBuffMap & operator=(const IOBuffMap &);
};*/


///
/// RW operation mapped to pure HTTP ops
class HttpIO{
public:
    HttpIO(Context & c, const Uri & uri, const RequestParams * params);
    virtual ~HttpIO();

    // full sequential read of a file from begining to the end
    ssize_t readFullBuff(void* buf, size_t count, DavixError** err);


    // execute a plain HTTP stat method for file info
    int stat(struct stat* st, DavixError** err);

    //
    void resetFullRead();

    // position independant read operation,
    // similar to pread except that does not need open() before
    ssize_t readPartialBuffer(void* buf, size_t count, off_t offset, DavixError** err);

    // position independant write operation,
    // similar to pwrite do not need open() before
    ssize_t putFullFromFD(const void* buf, size_t count, off_t offset, DavixError** err);

protected:
    Context & _c;
    Uri _uri;
    RequestParams _params;
    DppLock _rwlock;

    off_t _read_pos; //curent read file offset
    bool _read_endfile;

private:
    HttpRequest * _read_req;


    HttpIO(const HttpIO & );
    HttpIO & operator=(const HttpIO & );


};

///
/// RW operation with buffering support
/// add partial write support
class HttpIOBuffer : public HttpIO{
public:
    HttpIOBuffer(Context & c, const Uri & uri, const RequestParams * params);
    virtual ~HttpIOBuffer();

    // open the file associated with the davix IOBuffMap
    // do a simple check if the file exist and try to anticipate the next ops
    bool open(int flags, DavixError** err);

    //
    ssize_t read(void* buf, size_t count, DavixError** err);

    ssize_t pread(void* buf, size_t count, off_t offset, DavixError** err);

    //
    ssize_t write(const void* buf, size_t count, DavixError** err);


    //
    ssize_t pwrite(const void* buf, size_t count, off_t offset, DavixError** err);


    //
    off_t lseek(off_t offset, int flags, DavixError** err);

    // commit any pending operation on the file descriptor
    int commit(DavixError** err);

protected:

    size_t _file_size;
    bool _file_exist;
    off_t _pos;
    bool _opened;

private:

    HttpIOBuffer(const HttpIOBuffer & );
    HttpIOBuffer & operator=(const HttpIOBuffer & );
};

// create a single-ton filestream for cache from
int get_valid_cache_file(FILE** stream, DavixError** err);

} // namespace Davix

#endif // DAVIX_IOBUFFMAP_HPP
