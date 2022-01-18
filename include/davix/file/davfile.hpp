/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#ifndef DAVFILE_HPP
#define DAVFILE_HPP

#include <memory>
#include <istream>
#include <ostream>
#include <davixcontext.hpp>
#include <params/davixrequestparams.hpp>
#include <file/davix_file_info.hpp>
#include <compat/deprecated.hpp>



#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif


///
/// @file davfile.hpp
/// @author Devresse Adrien
///
///  File API of Davix


namespace Davix{


struct StatInfo;

///
/// @class DavFile
/// @brief Davix File Interface
///
/// Davix File interface
class DAVIX_EXPORT DavFile
{
public:
    struct DavFileInternal;

    class Iterator{
        friend struct DavFileInternal;
        public:
            Iterator() : d_ptr() {}
            Iterator(const Iterator & orig) : d_ptr(orig.d_ptr){}

            bool next();

            const std::string & name();
            const StatInfo & info();
        private:
           struct Internal;

           std::shared_ptr<Internal> d_ptr;
    };

    ///
    /// \brief default constructor
    /// \param c context
    /// \param url remote file URL
    ///
    DavFile(Context & c, const Uri & url);
    DavFile(Context &c, const RequestParams & params, const Uri &url);
    DavFile(const DavFile & orig);
    ///
    /// \brief destructor
    ///
    virtual ~DavFile();

    /// @brief return Uri of the current file
    ///
    const Uri & getUri() const;

    ///
    /// @brief return all replicas associated to this file
    ///
    /// Replicas are found using a corresponding The MetaLink standard ( rfc5854, rfc6249 )
    ///
    /// @param params  Davix Request parameters
    /// @param err  Davix error report
    /// @return  Replica vector, if error is found return empty vector and set err properly
    ///
    /// @snippet example_code_snippets.cpp getReplicas
    std::vector<DavFile> getReplicas(const RequestParams* params, DavixError** err) throw();

    ///
    ///  @brief Vector read operation
    ///        Able to do several read on several data chunk in one single operation.
    ///        Uses Http multi-part when supported by the server,
    ///        simulate a vector read operation otherwise.
    ///
    ///        NOTE: The return code is the number of data bytes received from the
    ///              server, not the total number of bytes written into the buffers.
    ///              The two might not be equal if range coalescing is performed.
    ///              Check diov_size of the output vector to make sure the buffers
    ///              contain the expected number of bytes.
    ///
    ///  @param params Davix request Parameters
    ///  @param input_vec input vectors, parameters
    ///  @param ioutput_vec  output vectors, results
    ///  @param count_vec  number of vector
    ///  @param err Davix error report
    ///  @return total number of bytes read, or -1 if error occures
    ///
    ///  @snippet example_code_snippets.cpp readPartialBufferVec
    dav_ssize_t readPartialBufferVec(const RequestParams* params,
                          const DavIOVecInput * input_vec,
                          DavIOVecOuput * ioutput_vec,
                          const dav_size_t count_vec,
                          DavixError** err) throw();

    ///
    ///  @brief Partial position independant read.
    ///
    ///
    ///         Use ranged request when supported by the server,
    ///         simulate a ranged request when not supported
    ///
    ///  @param params Davix request Parameters
    ///  @param buff  buffer
    ///  @param count  maximum read size
    ///  @param offset  starting offset for the read operation
    ///  @param err Davix error report
    ///  @return total number of bytes read, or -1 if error occures
    ///
    ///  @snippet example_code_snippets.cpp readPartial
    dav_ssize_t readPartial(const RequestParams* params,
                            void* buff,
                            dav_size_t count,
                            dav_off_t offset,
                            DavixError** err) throw();


    ///
    ///  @brief Get the full file content and write it to file descriptor
    ///
    ///  @param params Davix request Parameters
    ///  @param fd  file descriptor for write operation
    ///  @param err Davix error report
    ///  @return total number of bytes read, or -1 if error occures
    ///
    ///  @snippet example_code_snippets.cpp getToFd
    dav_ssize_t getToFd(const RequestParams* params,
                            int fd,
                            DavixError** err) throw();

    ///
    ///  @brief Get the first 'size_read' bytes of the file and write it to file descriptor
    ///
    ///  @param params Davix request Parameters
    ///  @param fd file descriptor for write operation
    ///  @param size_read number of bytes to read
    ///  @param err Davix error report
    ///  @return total number of bytes read, or -1 if error occures
    ///
    ///  @snippet example_code_snippets.cpp getToFd sized
    dav_ssize_t getToFd(const RequestParams* params,
                            int fd,
                            dav_size_t size_read,
                            DavixError** err) throw();

    ///
    ///  @brief Get the full file content in a dynamically allocated buffer
    ///
    ///  @param params Davix request Parameters
    ///  @param buffer reference to a vector for the result
    ///  @param err Davix error report
    ///  @return total number of bytes read, or -1 if error occures
    ///
    ///  @snippet example_code_snippets.cpp getFull
    dav_ssize_t getFull(const RequestParams* params,
                            std::vector<char> & buffer,
                            DavixError** err) throw();


    ///
    ///  @brief Get the full file content to buffer
    ///
    ///  @param params Davix request Parameters
    ///  @param buffer reference to a vector for storing the result
    ///  @return total number of bytes read, or -1 if error occures
    ///
    /// Get the file content in a dynamically allocated buffer
    ///
    ///  WARNING: this operation is without size limit for the content
    ///
    ///  @snippet example_code_snippets.cpp get
    dav_ssize_t get(const RequestParams* params,
                    std::vector<char> & buffer);



    ///
    ///  @brief Create/Replace file content
    ///
    ///  @param params Davix request Parameters
    ///  @param fd file descriptor
    ///  @param size_write number of bytes to write
    ///  @throw throw @ref DavixException if an error occurs
    ///
    ///  Create / Replace the file.
    ///  Read the new content from the file descriptor fd for a maximum of size_write bytes.
    ///
    ///  @snippet example_code_snippets.cpp put fd
    void put(const RequestParams* params, int fd, dav_size_t size_write);


    ///
    ///  @brief Create/Replace file content
    ///
    ///  @param params Davix request Parameters
    ///  @param buffer buffer with data to write
    ///  @param size_write number of bytes to write
    ///  @throw throw @ref DavixException if an error occurs
    ///
    ///  Set a new content for the file.
    ///  The new content comes from buffer.
    ///
    ///  @snippet example_code_snippets.cpp put buffer
    void put(const RequestParams* params, const char* buffer, dav_size_t size_write);

#ifdef __DAVIX_HAS_STD_FUNCTION

    ///
    ///  @brief Create/Replace file content
    ///
    ///  @param params Davix request Parameters
    ///  @param callback data provider callback
    ///  @param size Total size of the data to write
    ///  @throw throw @ref DavixException if an error occurs
    ///
    ///  Set a new content for the file.
    ///  The new content comes from a data provider callback.
    ///
    ///  @snippet example_code_snippets.cpp put callback
    void put(const RequestParams* params, const DataProviderFun & callback, dav_size_t size);

#endif

    ///
    /// @brief move
    /// @param params Davix request Parameters
    /// @param destination destination resource
    ///
    /// Move the current resource to Destination.
    ///
    /// The result of the operation depend of the protocol used.
    ///
    /// Protocol supported currently: WebDav, S3
    ///
    /// @snippet example_code_snippets.cpp move
    void move(const RequestParams* params, DavFile & destination);


    ///
    ///  @brief Suppress the current entity or collection.
    ///
    ///  @param params Davix request Parameters
    ///  @throw  throw @ref DavixException if error occurs
    ///
    ///  @snippet example_code_snippets.cpp delete
    void deletion(const RequestParams* params = NULL);

    ///
    ///  @brief Suppress the current entity or collection.
    ///
    ///  Exception safe version of @ref deletion(const RequestParams* params = NULL)
    ///
    ///  @snippet example_code_snippets.cpp delete no throw
    int deletion(const RequestParams* params,
                 DavixError** err) throw();


    ///
    ///  @brief create a collection (directory or bucket) at the current url
    ///
    ///  @param params Davix request Parameters
    ///  @throw  throw @ref DavixException if error occurs
    ///
    ///  @snippet example_code_snippets.cpp makeCollection
    void makeCollection(const RequestParams *params = NULL);

    ///
    ///  @brief create a collection (directory or bucket) at the current url
    ///
    ///  Exception safe version of @ref makeCollection(const RequestParams *params = NULL)
    ///
    ///  @snippet example_code_snippets.cpp makeCollection no throw
    int makeCollection(const RequestParams* params,
                       DavixError** err) throw();

    ///
    ///  @brief execute a file meta-data query
    ///
    ///  @param params Davix request Parameters
    ///  @param info stat struct
    ///  @return 0 if success, or -1 if error occures
    ///
    ///  @snippet example_code_snippets.cpp statInfo
    StatInfo & statInfo(const RequestParams* params, StatInfo & info);

    ///
    ///  @brief execute a POSIX-like stat() query
    ///
    ///  @param params Davix request parameters
    ///  @param st stat struct
    ///  @param err Davix error report
    ///  @return 0 if success, or -1 if error occures
    ///
    ///  @snippet example_code_snippets.cpp stat
    int stat(const RequestParams* params, struct stat * st, DavixError** err) throw();

    ///
    ///  @brief Collection listing
    ///
    ///  @param params Davix request parameters
    ///  @return Iterator to the collection
    ///
    ///  @snippet example_code_snippets.cpp listCollection
    Iterator  listCollection(const RequestParams* params);


    ///
    ///  @brief compute checksum of the file
    ///
    ///  with the given algorithm (MD5, CRC32, ADLER32)
    ///
    ///  server implementation dependend
    ///
    ///  Davix::checksum support LCGDM-DAV, dCache Jetty and Aws S3 checksum support
    ///
    ///  @param params request parameters
    ///  @param checksm checksum buffer
    ///  @param chk_algo string of the algorithm (eg: "MD5"  )
    ///  @return reference to checksm
    ///  @throw  throw @ref DavixException if error occurs
    ///
    ///  @snippet example_code_snippets.cpp checksum
    std::string & checksum(const RequestParams *params, std::string &checksm, const std::string &chk_algo);

    ///
    ///  @brief compute checksum of the file with the given algorithm (MD5, CRC32, ADLER32)
    ///
    ///  Exception safe version of @ref checksum
    ///
    ///  @snippet example_code_snippets.cpp checksum no throw
    int checksum(const RequestParams *params, std::string & checksm, const std::string & chk_algo, DavixError **err) throw();


    ///
    /// @brief provide information on the next file operation
    ///
    /// provide information on the next file operations for optimizations and prefetching
    ///
    /// @param offset
    /// @param size_read
    /// @param adv
    ///
    void prefetchInfo(off_t offset, dav_size_t size_read, advise_t adv);

    ///
    /// @brief retrieve quota information
    ///
    /// retrieve quota information about a directory
    ///
    /// @param params
    /// @param info
    ///
    QuotaInfo & quotaInfo(const RequestParams* params, QuotaInfo & info);

private:
    DavFileInternal* d_ptr;

public:
    /// @deprecated deprecated, will be removed in 2.0
    DEPRECATED(dav_ssize_t getAllReplicas(const RequestParams* params,
                                    ReplicaVec & vec, DavixError** err));

    ///
    ///  @deprecated please use put() as the replacement, will be removed in 2.0
    ///
    DEPRECATED(int putFromFd(const RequestParams* params,
                  int fd,
                  dav_size_t size_write,
                  DavixError** err) throw());
};

typedef DavFile File;


} // Davix


// provide stream operator for Davix::File
std::ostream & operator<<(std::ostream & out, Davix::DavFile & file);
std::istream & operator>>(std::istream & in, Davix::DavFile & file);

#endif // DAVFILE_HPP
