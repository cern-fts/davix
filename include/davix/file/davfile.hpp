/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) 2013  Adrien Devresse <adrien.devresse@cern.ch>, CERN
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
            Iterator() : d_ptr(NULL) {}
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
    /// \param url Remote File URL
    ///
    DavFile(Context & c, const Uri & url);
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
    /// @param err  DavixError error report
    /// @return  Replica vector, if error is found return empty vector and set err properly
    std::vector<DavFile> getReplicas(const RequestParams* params, DavixError** err);

    ///
    ///  @brief Vector read operation
    ///        Allow to do several read several data chunk in one single operation
    ///        Use Http multi-part when supported by the server,
    ///        simulate a vector read operation in the other case
    ///
    ///  @param params Davix request Parameters
    ///  @param input_vec input vectors, parameters
    ///  @param ioutput_vec  output vectors, results
    ///  @param count_vec  number of vector struct
    ///  @param err Davix Error report
    ///  @return total number of bytes read, or -1 if error occures
    ///
    dav_ssize_t readPartialBufferVec(const RequestParams* params,
                          const DavIOVecInput * input_vec,
                          DavIOVecOuput * ioutput_vec,
                          const dav_size_t count_vec,
                          DavixError** err);

    ///
    ///  @brief Partial position independant read.
    ///
    ///
    ///         Use Ranged request when supported by the server,
    ///         simulate a ranged request when not  supported
    ///
    ///  @param params Davix request Parameters
    ///  @param buff  buffer
    ///  @param count  maximum read size
    ///  @param offset  start offset  for the read operation
    ///  @param err Davix Error report
    ///  @return total number of bytes read, or -1 if error occures
    ///
    dav_ssize_t readPartial(const RequestParams* params,
                            void* buff,
                            dav_size_t count,
                            dav_off_t offset,
                            DavixError** err);


    ///
    ///  @brief Get the full file content and write it to fd
    ///
    ///  @param params Davix request Parameters
    ///  @param fd  file descriptor for write operation
    ///  @param err Davix Error report
    ///  @return total number of bytes read, or -1 if error occures
    ///
    dav_ssize_t getToFd(const RequestParams* params,
                            int fd,
                            DavixError** err);

    ///
    ///  @brief Get the first 'size_read' bytes of the file and write it to fd
    ///
    ///  @param params Davix request Parameters
    ///  @param fd file descriptor for write operation
    ///  @param size_read number of bytes to read
    ///  @param err Davix Error report
    ///  @return total number of bytes read, or -1 if error occures
    ///
    dav_ssize_t getToFd(const RequestParams* params,
                            int fd,
                            dav_size_t size_read,
                            DavixError** err);

    ///
    ///  @brief Get the full file content in a dynamically allocated buffer
    ///
    ///  @param params Davix request Parameters
    ///  @param buffer reference to a vector for the result
    ///  @param err Davix Error report
    ///  @return total number of bytes read, or -1 if error occures
    ///
    dav_ssize_t getFull(const RequestParams* params,
                            std::vector<char> & buffer,
                            DavixError** err);

    ///
    ///  @brief create and replace the file with the content
    ///     of the file descriptor fd
    ///
    ///  @param params Davix request Parameters
    ///  @param fd file descriptor
    ///  @param size_write number of bytes to write    
    ///  @param err Davix Error report
    ///  @return 0 if success, or -1 if error occures
    ///
    int putFromFd(const RequestParams* params,
                  int fd,
                  dav_size_t size_write,
                  DavixError** err);



    ///
    ///  @brief Suppress the current entity.
    ///         able to suppress collection too
    ///
    ///  @param params Davix request Parameters
    ///  @throw  throw @ref DavixException if error occurs
    void deletion(const RequestParams* params = NULL);

    ///
    ///  @brief Suppress the current entity.
    ///         able to suppress collection too
    ///  Exception safe version of @ref deletion(const RequestParams* params = NULL)
    int deletion(const RequestParams* params,
                 DavixError** err) throw();


    ///
    /// @brief create a collection ( directory or bucket) at the current url
    ///
    ///  @param params Davix request Parameters
    ///  @throw  throw @ref DavixException if error occurs
    ///
    void makeCollection(const RequestParams *params = NULL);

    ///
    ///  @brief create a collection ( directory or bucket) at the current url
    ///
    ///  Exception safe version of @ref makeCollection(const RequestParams *params = NULL)
    int makeCollection(const RequestParams* params,
                       DavixError** err) throw();

    ///
    /// @brief execute a file meta-data query
    ///
    ///  @param params Davix request Parameters
    ///  @param st stat struct
    ///  @param err Davix Error report
    ///  @return 0 if success, or -1 if error occures
    ///
    StatInfo & statInfo(const RequestParams* params, StatInfo & info);

    ///
    /// @brief execute a POSIX-like stat() query
    ///
    ///  @param params DaviX request parameters
    ///  @param st stat struct
    ///  @param err Davix Error report
    ///  @return 0 if success, or -1 if error occures
    ///
    int stat(const RequestParams* params, struct stat * st, DavixError** err);

    ///
    /// @brief Collection listing
    ///
    ///  @param params DaviX request parameters
    ///  @return Iterator to the collection
    Iterator  listCollection(const RequestParams* params);


    ///
    /// @brief compute checksum of the file
    /// with the given algorithm (MD5, CRC32, ADLER32)
    ///
    ///  server implementation dependend
    ///  Davix::checksum support LCGDM-DAV, dCache Jetty and Aws S3 checksum support
    ///
    /// @param params request parameters
    /// @param checksm checksum buffer
    /// @param chk_algo string of the algorithm (eg: "MD5"  )
    /// @return reference to checksm, throw DavixException if error occurs
    std::string & checksum(const RequestParams *params, std::string &checksm, const std::string &chk_algo);

    ///
    /// @brief compute checksum of the file with the given algorithm (MD5, CRC32, ADLER32)
    ///
    /// Exception safe version of @ref checksum
    int checksum(const RequestParams *params, std::string & checksm, const std::string & chk_algo, DavixError **err) throw();

    ///
    /// @brief provide informations on the next file operation
    ///
    /// provide informations on the next file operations for optimizations
    /// and prefetching
    ///
    /// @param offset
    /// @param size_read
    /// @param adv
    ///
    void prefetchInfo(off_t offset, dav_size_t size_read, advise_t adv);

private:
    DavFileInternal* d_ptr;

public:
    /// @deprecated deprecated, will be removed in 1.0
    dav_ssize_t getAllReplicas(const RequestParams* params,
                                    ReplicaVec & vec, DavixError** err);
};

typedef DavFile File;


} // Davix

#endif // DAVFILE_HPP
