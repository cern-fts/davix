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

#include <utils/davix_logger_internal.hpp>

#include <status/davixstatusrequest.hpp>
#include <fileops/chain_factory.hpp>
#include <xml/davpropxmlparser.hpp>
#include <utils/stringutils.hpp>
#include <file/davposix.hpp>

using namespace StrUtil;



namespace Davix{

const std::string scope = "DavFile";

static HttpIOChain & getIOChain(HttpIOChain & chain){
    CreationFlags flags;
    flags[CHAIN_POSIX] = true;

    return ChainFactory::instanceChain(flags, chain);
}

static IOChainContext  getIOContext( Context & context, const Uri & uri, const RequestParams* params){
        return IOChainContext(context, uri, params);
}

}

// static const std::string simple_listing("<propfind xmlns=\"DAV:\"><prop><resourcetype><collection/></resourcetype></prop></propfind>");
//
// static const std::string stat_listing("<?xml version=\"1.0\" encoding=\"utf-8\" ?><D:propfind xmlns:D=\"DAV:\" xmlns:L=\"LCGDM:\"><D:prop>"
//                                       "<D:displayname/><D:getlastmodified/><D:creationdate/><D:getcontentlength/><D:quota-used-bytes/>"
//                                       "<D:resourcetype><D:collection/></D:resourcetype><L:mode/>"
//                                       "</D:prop>"
//                                       "</D:propfind>");

struct Davix_dir_handle{

    Davix_dir_handle(Davix::Context & context, const Davix::Uri & u, const Davix::RequestParams * p):
        params(p),
        uri(u),
        io_chain(),
        io_context(context, uri, &params),
        start_entry_name(),
        start_entry_st(),
        dir_info((struct dirent*) calloc(1,sizeof(struct dirent) + NAME_MAX +1)),
        dir_offset(0),
        end(false){
        Davix::getIOChain(io_chain);
    }

   ~Davix_dir_handle(){
        free(dir_info);
    }


    // params
    Davix::RequestParams params;
    Davix::Uri uri;

    Davix::HttpIOChain io_chain;
    Davix::IOChainContext io_context;

   //first entry
   std::string start_entry_name;
   Davix::StatInfo start_entry_st;

   // dir struct
   struct dirent* dir_info;
   off_t dir_offset;

   // end listing
   bool end;

private:
   Davix_dir_handle(const Davix_dir_handle & );
   Davix_dir_handle & operator=(const Davix_dir_handle & );
};

struct Davix_fd{
    Davix_fd(Davix::Context & context, const Davix::Uri & uri, const Davix::RequestParams * params) : _uri(uri), _params(params),
        io_handler(), io_context(Davix::getIOContext(context, _uri, &_params)){
        Davix::getIOChain(io_handler);
    }
    virtual ~Davix_fd(){
        try{
            io_handler.resetIO(io_context);
        }catch(Davix::DavixException & e){
            DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_POSIX, "Error when closed file descriptor, possibly file corrupted {}", e.what());
        }
    }

    Davix::Uri _uri;
    Davix::RequestParams _params;
    Davix::HttpIOChain io_handler;
    Davix::IOChainContext io_context;
};


namespace Davix {



static void toDirent(struct dirent * d, const std::string & filename, const StatInfo & info){
    StrUtil::copy_std_string_to_buff(d->d_name, NAME_MAX, filename);
    if (S_ISDIR(info.mode))
        d->d_type = DT_DIR;
    else if (S_ISLNK(info.mode))
        d->d_type = DT_LNK;
    else
        d->d_type = DT_REG;
}



DavPosix::DavPosix(Context* _context) :
    context(_context),
    _timeout(180),
    _s_buff(2048),
    d_ptr(NULL)

{
    (void ) d_ptr; // silence warning
}

DavPosix::~DavPosix(){

}

DAVIX_DIR* internal_opendir(Context & context, const RequestParams* params, const std::string & url){
    std::unique_ptr<DAVIX_DIR> dir(new DAVIX_DIR(context, url, params));
    dir->end = ! dir->io_chain.nextSubItem(dir->io_context,dir->start_entry_name, dir->start_entry_st);
    return dir.release();
}


DAVIX_DIR* DavPosix::opendir(const RequestParams* params, const std::string &url, DavixError** err){

    DAVIX_SCOPE_TRACE(DAVIX_LOG_POSIX, odir);

    DAVIX_DIR* r  = NULL;
    TRY_DAVIX{
        r = internal_opendir(*context, params, url);
    }CATCH_DAVIX(err)

    return (DAVIX_DIR*) r;
}

DAVIX_DIR* DavPosix::opendirpp(const RequestParams* params, const std::string &url, DavixError** err){

    DAVIX_SCOPE_TRACE(DAVIX_LOG_POSIX, odirpp);

    DAVIX_DIR* r  = NULL;

    TRY_DAVIX{
        r = internal_opendir(*context, params, url);
    }CATCH_DAVIX(err)

    return (DAVIX_DIR*) r;
}


struct dirent* internal_readdir(DAVIX_DIR * dir, struct stat* st){
    if( dir==NULL){
        throw DavixException(davix_scope_directory_listing_str(), StatusCode::InvalidFileHandle,  "Invalid file descriptor for DAVIX_DIR*");
    }

    if(dir->end)
        return NULL;

    if(dir->start_entry_name.size() >0) { // first entry
        if(st){
            dir->start_entry_st.toPosixStat(*st);
        }
        toDirent(dir->dir_info, dir->start_entry_name, dir->start_entry_st);
        dir->start_entry_name.clear();
        return dir->dir_info;
    }

    StatInfo info;
    std::string name;
    if( dir->io_chain.nextSubItem(dir->io_context, name, info) ==false){
        return NULL;
    }

    if(st){
         info.toPosixStat(*st);
    }
    toDirent(dir->dir_info, name, info);
    return dir->dir_info;

}


struct dirent* DavPosix::readdir(DAVIX_DIR * d, DavixError** err){
    DAVIX_SCOPE_TRACE(DAVIX_LOG_POSIX, rdir);

    DavixError* tmp_err=NULL;
    DAVIX_DIR* dir = static_cast<DAVIX_DIR*>(d);

    TRY_DAVIX{
        return internal_readdir(dir, NULL);
    }CATCH_DAVIX(&tmp_err)

    DavixError::propagateError(err, tmp_err);
    return NULL;
}

struct dirent* DavPosix::readdirpp(DAVIX_DIR * d, struct stat *st, DavixError** err){
    DAVIX_SCOPE_TRACE(DAVIX_LOG_POSIX, rdirpp);

    DavixError* tmp_err=NULL;
    DAVIX_DIR* dir = static_cast<DAVIX_DIR*>(d);

    TRY_DAVIX{
        return internal_readdir(dir, st);
    }CATCH_DAVIX(&tmp_err)

    DavixError::propagateError(err, tmp_err);
    return NULL;
}


int DavPosix::closedirpp(DAVIX_DIR * d, DavixError** err){
    int ret =-1;

    TRY_DAVIX{
        if( d==NULL){
            throw DavixException(davix_scope_directory_listing_str(), Davix::StatusCode::InvalidFileHandle, "Invalid file descriptor for DAVIX_DIR*");
       }else{
            delete (static_cast<DAVIX_DIR*>(d));
            ret =0;
        }

    }CATCH_DAVIX(err)

    return ret;
}

int DavPosix::closedir(DAVIX_DIR * d, DavixError** err){
    return closedirpp(d,err);
}

void DavPosix::fadvise(DAVIX_FD *fd, dav_off_t offset, dav_size_t len, advise_t advise){
    try{
        if( fd==NULL)
            return;
        fd->io_handler.prefetchInfo(fd->io_context, offset, len, advise);

    }catch(DavixException & e){
        DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_POSIX, "fdadvise error {}", e.what());
    }catch(...){
        DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_POSIX, "Unknown error, aborted");
    }
}


////////////////////////////////////////////////////
//////////////////// Davix POSIX meta ops
/////////////////////////////////////////////////////

int DavPosix::rename(const RequestParams * _params, const std::string &source_url, const std::string &target_url, DavixError** err){

    DAVIX_SCOPE_TRACE(DAVIX_LOG_POSIX, renm);
    int ret=-1;

    TRY_DAVIX{
        Uri uri(source_url);
        HttpIOChain chain;
        IOChainContext io_context = getIOContext(*context, uri, _params);

        getIOChain(chain).move(io_context, target_url);
        ret = 0;
    }CATCH_DAVIX(err)

    return ret;
}



int DavPosix::mkdir(const RequestParams * _params, const std::string &url, mode_t right, DavixError** err){
    DAVIX_SCOPE_TRACE(DAVIX_LOG_POSIX, fun_mkdir);
    (void) right;
    int ret=-1;

    TRY_DAVIX{
        Uri uri(url);
        HttpIOChain chain;
        IOChainContext io_context = getIOContext(*context, uri, _params);

        getIOChain(chain).makeCollection(io_context);
        ret = 0;
    }CATCH_DAVIX(err)
    return ret;
}


int DavPosix::stat(const RequestParams * params, const std::string & url, struct stat* st, DavixError** err){
    DAVIX_SCOPE_TRACE(DAVIX_LOG_POSIX, fun_stat);

    File f(*context, url);
    int ret = f.stat(params, st, err);

    return ret;

}

int DavPosix::stat64(const RequestParams *params, const std::string & url, StatInfo *st, DavixError **err){
    TRY_DAVIX{
        if(st == NULL)
            throw DavixException(davix_scope_meta(), StatusCode::InvalidArgument, "Argument stat NULL");

        File f(*context, url);
        f.statInfo(params, *st);
        return 0;
    }CATCH_DAVIX(err)
    return -1;
}

int DavPosix::get_quota(const RequestParams *params, const std::string & url, QuotaInfo *info, DavixError **err) {
    TRY_DAVIX{
        if(info == NULL)
            throw DavixException(davix_scope_meta(), StatusCode::InvalidArgument, "Argument info is NULL");

        File f(*context, url);
        f.quotaInfo(params, *info);
        return 0;
    }CATCH_DAVIX(err)
    return -1;
}

int davix_remove_posix(Context* context, const RequestParams * params, const std::string & url, bool directory, DavixError** err){
    DavixError* tmp_err = NULL;
    int ret = -1;
    Uri uri(url);

    TRY_DAVIX{
        if(params && params->getProtocol() == RequestProtocol::Http){ // pure protocol http : ignore posix semantic, execute a simple delete
            HttpIOChain chain;
            IOChainContext io_context = getIOContext(*context, uri, params);
            getIOChain(chain).deleteResource(io_context);
            ret = 0;
        }else{
            // full posix semantic support
            HttpIOChain chain;
            IOChainContext io_context = getIOContext(*context, uri, params);
            struct StatInfo infos;

            getIOChain(chain).statInfo(io_context, infos);

            if( S_ISDIR(infos.mode)){ // directory : impossible to delete if not empty
                if(directory == true){
                    chain.deleteResource(io_context);
                    ret =0;
                }else{
                    ret = -1;
                    throw DavixException(davix_scope_davOps_str(), StatusCode::IsADirectory, fmt::format(" {} is a directory, impossible to unlink\n", uri));
                }
            }else{ // file, rock & roll
                if(directory == false){
                    chain.deleteResource(io_context);
                    ret =0;
                }else{
                    ret = -1;
                    throw DavixError(davix_scope_davOps_str(), StatusCode::IsNotADirectory, fmt::format(" {} is not a directory, impossible to rmdir", uri));
                }
            }
        }
    }CATCH_DAVIX(&tmp_err)

    DavixError::propagateError(err, tmp_err);
    return ret;

}


int DavPosix::unlink(const RequestParams * params, const std::string &uri, DavixError** err){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_POSIX, " -> davix_unlink");
    int ret=-1;
    DavixError* tmp_err=NULL;

    TRY_DAVIX{
        ret = davix_remove_posix(context, params, uri, false, &tmp_err);
    }CATCH_DAVIX(&tmp_err)

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_POSIX, " davix_unlink <-");
    DavixError::propagatePrefixedError(err, tmp_err, "DavPosix::unlink ");
    return ret;
}


int DavPosix::rmdir(const RequestParams * params, const std::string &uri, DavixError** err){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_POSIX, " -> davix_rmdir");
    int ret=-1;
    DavixError* tmp_err=NULL;

    TRY_DAVIX{
        ret = davix_remove_posix(context, params, uri, true, &tmp_err);
    }CATCH_DAVIX(&tmp_err)

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_POSIX, " davix_rmdir <-");
    DavixError::propagatePrefixedError(err, tmp_err, "DavPosix::rmdir ");
    return ret;
}



////////////////////////////////////////////////////
//////////////////// Davix POSIX I/O
/////////////////////////////////////////////////////



inline int davix_check_rw_fd(DAVIX_FD* fd, DavixError** err){
    if(fd == NULL){
        DavixError::setupError(err, davix_scope_http_request(),StatusCode::InvalidFileHandle, "Invalid Davix file descriptor");
        return -1;
    }
    return 0;
}


DAVIX_FD* DavPosix::open(const RequestParams * _params, const std::string & url, int flags, DavixError** err){
    DAVIX_SCOPE_TRACE(DAVIX_LOG_POSIX, fun_open);
    DavixError* tmp_err=NULL;
    Davix_fd* fd = NULL;


    TRY_DAVIX{
        Uri uri(url);

        if(uri.getStatus() != StatusCode::OK){
            throw DavixException(davix_scope_http_request(), uri.getStatus(), " Uri invalid in Davix::Open");
        }
        fd = new Davix_fd(*context, uri, _params);
        fd->io_handler.open(fd->io_context, flags);
    }CATCH_DAVIX(&tmp_err)

    if(tmp_err){
        DavixError::propagateError(err, tmp_err);
        delete fd;
        fd= NULL;
    }
    return fd;
}


ssize_t DavPosix::read(DAVIX_FD* fd, void* buf, size_t count, Davix::DavixError** err){
    DAVIX_SCOPE_TRACE(DAVIX_LOG_POSIX, fun_read);

    ssize_t ret =-1;
    DavixError* tmp_err=NULL;

    TRY_DAVIX{
        if( davix_check_rw_fd(fd, &tmp_err) ==0){
            ret = (ssize_t) fd->io_handler.read(fd->io_context, buf, (dav_size_t) count);
        }
    }CATCH_DAVIX(&tmp_err)

    DavixError::propagateError(err, tmp_err);
    return ret;
}


ssize_t DavPosix::pread(DAVIX_FD* fd, void* buf, size_t count, off_t offset, DavixError** err){
    return static_cast<ssize_t>(pread64(fd, buf, static_cast<dav_size_t>(count), static_cast<dav_off_t>(offset), err));
}

dav_ssize_t DavPosix::pread64(DAVIX_FD *fd, void *buf, dav_size_t count, dav_off_t offset, DavixError **err){
    DAVIX_SCOPE_TRACE(DAVIX_LOG_POSIX, fun_pread);
    dav_ssize_t ret =-1;
    DavixError* tmp_err=NULL;

    TRY_DAVIX{
        if( davix_check_rw_fd(fd, &tmp_err) ==0){
            ret = fd->io_handler.pread(fd->io_context, buf, count, offset);
        }
    }CATCH_DAVIX(&tmp_err)

    DavixError::propagateError(err, tmp_err);;
    return ret;
}

ssize_t DavPosix::pwrite(DAVIX_FD* fd, const void* buf, size_t count, off_t offset, DavixError** err){
    (void) fd;
    (void) buf;
    (void) count;
    (void) offset;
    DAVIX_SCOPE_TRACE(DAVIX_LOG_POSIX, fun_pwrite);
    DavixError::setupError(err, davix_scope_io_buff(), StatusCode::OperationNonSupported, "Operation pwrite Not supported");
    return -1;
}

dav_ssize_t DavPosix::pwrite64(DAVIX_FD *fd, const void *buf, dav_size_t count, dav_off_t offset, DavixError **err){
    (void) fd;
    (void) buf;
    (void) count;
    (void) offset;
    DAVIX_SCOPE_TRACE(DAVIX_LOG_POSIX, fun_pwrite);
    DavixError::setupError(err, davix_scope_io_buff(), StatusCode::OperationNonSupported, "Operation pwrite Not supported");
    return -1;
}

dav_ssize_t DavPosix::preadVec(DAVIX_FD* fd, const DavIOVecInput * input_vec,
                      DavIOVecOuput * output_vec,
                      dav_size_t count_vec, DavixError** err){
    DAVIX_SCOPE_TRACE(DAVIX_LOG_POSIX, fun_preadvec);
    ssize_t ret =-1;
    DavixError* tmp_err=NULL;

    TRY_DAVIX{
        if( davix_check_rw_fd(fd, &tmp_err) ==0){
            ret = fd->io_handler.preadVec(fd->io_context, input_vec, output_vec, count_vec);
        }
    }CATCH_DAVIX(&tmp_err)

    DavixError::propagateError(err, tmp_err);
    return ret;
}

ssize_t DavPosix::write(DAVIX_FD* fd, const void* buf, size_t count, Davix::DavixError** err){
    DAVIX_SCOPE_TRACE(DAVIX_LOG_POSIX, fun_write);
    ssize_t ret =-1;
    DavixError* tmp_err=NULL;

    TRY_DAVIX{
        if( davix_check_rw_fd(fd, &tmp_err) ==0){
            // BufferContentProvider provider( (const char*) buf, count);
            // ret = (ssize_t) fd->io_handler.writeFromProvider(fd->io_context, provider);
            ret = (ssize_t) fd->io_handler.write(fd->io_context, buf, (dav_size_t) count);
        }
    }CATCH_DAVIX(&tmp_err)

    DavixError::propagateError(err, tmp_err);
    return ret;
}



off_t DavPosix::lseek(DAVIX_FD* fd, off_t offset, int flags, Davix::DavixError** err){
   dav_off_t res = lseek64(fd, static_cast<dav_off_t>(offset), flags, err);
   if(res > std::numeric_limits<off_t>::max())
       return std::numeric_limits<off_t>::max();
   return res;
}

dav_off_t DavPosix::lseek64(DAVIX_FD *fd, dav_off_t offset, int flags, DavixError **err){
    DAVIX_SCOPE_TRACE(DAVIX_LOG_POSIX, fun_lseek);
    ssize_t ret =-1;
    DavixError* tmp_err=NULL;

    TRY_DAVIX{
        if( davix_check_rw_fd(fd, &tmp_err) ==0){
            ret = fd->io_handler.lseek(fd->io_context, offset, flags);
        }
    }CATCH_DAVIX(&tmp_err)

    DavixError::propagateError(err, tmp_err);
    return ret;
}


int DavPosix::close(DAVIX_FD* fd, Davix::DavixError** err){
    TRY_DAVIX{
        if(fd){
            fd->io_handler.resetIO(fd->io_context);
            delete fd;
            fd = NULL;
        }
    }CATCH_DAVIX(err)
    return 0;
}


} // namespace Davix
