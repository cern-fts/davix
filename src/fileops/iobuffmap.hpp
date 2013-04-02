#pragma once
#ifndef DAVIX_IOBUFFMAP_HPP
#define DAVIX_IOBUFFMAP_HPP

#include <memory/memoryutils.hpp>
#include <davix.hpp>
#include <libs/lockers/dpplocker.hpp>
#include <fileops/fileutils.hpp>



namespace Davix {

//
// Internal POSIX like to HTTP RW operation mapper
// provides facilities for caching
//


/// RW operation mapped to pure HTTP ops
class HttpIO{
public:
    HttpIO(Context & c, const Uri & uri, const RequestParams * params);
    virtual ~HttpIO();

    // full sequential read of a file from begining to the end
    ssize_t readFullBuff(void* buf, size_t count, DavixError** err);

    // read to dynamically allocated buffer
    dav_ssize_t readFull(std::vector<char> & buffer, DavixError** err);

    // read to dynamically allocated buffer
    dav_ssize_t readFull(std::string & buffer , DavixError** err);


    // execute a plain HTTP stat method for file info
    int stat(struct stat* st, DavixError** err);
    int stat(struct stat* st, HttpCacheToken** token, DavixError** err);

    //
    void resetFullRead();

    // position independant read operation,
    // similar to pread except that does not need open() before
    ssize_t readPartialBuffer(void* buf, size_t count, off_t offset, DavixError** err);

    // vec read
    dav_ssize_t readPartialBufferVec(const DavIOVecInput * input_vec,
                          DavIOVecOuput * ioutput_vec,
                          const dav_size_t count_vec, DavixError** err);

    // read to fd
    dav_ssize_t readToFd(int fd, dav_size_t size, DavixError** err);



    // position independant write operation,
    // similar to pwrite do not need open() before
    ssize_t writeFullFromFd(int fd, dav_size_t size, DavixError** err);

protected:
    Context & _c;
    Uri _uri;
    RequestParams _params;
    DppLock _rwlock;

    off_t _read_pos; //curent read file offset
    bool _read_endfile;

    // cache token
    ScopedPtr<HttpCacheToken>::type _token;
private:
    HttpRequest * _read_req;


    HttpIO(const HttpIO & );
    HttpIO & operator=(const HttpIO & );


};

///
/// RW operation with buffering support
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

    dav_ssize_t preadVec(const DavIOVecInput * input_vec,
                          DavIOVecOuput * ioutput_vec,
                          dav_size_t count_vec, DavixError** err);


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


ssize_t read_segment_request(HttpRequest* req, void* buffer, size_t size_read,  off_t off_set, DavixError**err);


} // namespace Davix

#endif // DAVIX_IOBUFFMAP_HPP
