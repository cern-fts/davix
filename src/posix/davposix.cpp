#include <config.h>
#include <ostream>
#include <sstream>

#include <memory/memoryutils.hpp>
#include <logger/davix_logger_internal.h>
#include <status/davixstatusrequest.hpp>
#include <fileops/fileutils.hpp>
#include <fileops/fileutils.hpp>
#include <fileops/davops.hpp>
#include <fileops/davmeta.hpp>
#include <xml/davpropxmlparser.hpp>
#include <fileops/iobuffmap.hpp>
#include <string_utils/stringutils.hpp>
#include <posix/davposix.hpp>


static const std::string simple_listing("<propfind xmlns=\"DAV:\"><prop><resourcetype><collection/></resourcetype></prop></propfind>");

static const std::string stat_listing("<?xml version=\"1.0\" encoding=\"utf-8\" ?><D:propfind xmlns:D=\"DAV:\" xmlns:L=\"LCGDM:\"><D:prop>"
                                      "<D:displayname/><D:getlastmodified/><D:creationdate/><D:getcontentlength/>"
                                      "<D:resourcetype><D:collection/></D:resourcetype><L:mode/>"
                                      "</D:prop>"
                                      "</D:propfind>");



struct Davix_fd{
    Davix_fd(Davix::HttpIOBuffer * buff) : io_handler(buff){}
    ScopedPtr<Davix::HttpIOBuffer>::type io_handler;
};


struct Davix_dir_handle{

    Davix_dir_handle(Davix::HttpRequest* _req, Davix::DavPropXMLParser * _parser):
        request(_req),
        parser(_parser),
        dir_info((struct dirent*) calloc(1,sizeof(struct dirent) + NAME_MAX +1)),
        dir_offset(0){

    }

   ~Davix_dir_handle(){
        delete request;
        delete parser;
        free(dir_info);
    }

   Davix::HttpRequest* request;
   Davix::DavPropXMLParser * parser;
   struct dirent* dir_info;
   off_t dir_offset;

private:
   Davix_dir_handle(const Davix_dir_handle & );
   Davix_dir_handle & operator=(const Davix_dir_handle & );
};



typedef Davix_dir_handle DIR_handle;

namespace Davix {

DavPosix::DavPosix(Context* _context) :
    context(_context),
    _timeout(180),
    _s_buff(2048),
    d_ptr(NULL)

{

}

DavPosix::~DavPosix(){

}




static void fill_dirent_from_filestat(struct dirent * d, const FileProperties & f){
    copy_std_string_to_buff(d->d_name, NAME_MAX, f.filename);
}

int incremental_propfind_listdir_parsing(HttpRequest* req, DavPropXMLParser * parser, size_t s_buff, const char* scope, DavixError** err){
  //  std::cout << "time 1 pre-fecth" << time(NULL) << std::endl;
    DavixError* tmp_err=NULL;

    char buffer[s_buff+1];
    const ssize_t ret_s_buff= req->readBlock(buffer, s_buff, &tmp_err);
    if(ret_s_buff >= 0){
        buffer[ret_s_buff]= '\0';
        DAVIX_DEBUG("chunk parse : result content : %s", buffer);
        if( parser->parseChuck(buffer, ret_s_buff) < 0){
            DavixError::propagateError(err, parser->getLastErr());
            return -1;
        }
    }

    //std::cout << "time 3 post-parse" << time(NULL) << std::endl;
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret_s_buff;
}




void configure_req_for_listdir(HttpRequest* req){
    req->addHeaderField("Depth","1");
}

DAVIX_DIR* DavPosix::internal_opendirpp(const RequestParams* _params, const char * scope, const std::string & body, const std::string & url, DavixError** err ){
    ssize_t s_resu;
    DAVIX_DIR* r = NULL;
    int ret =-1;
    DavixError* tmp_err=NULL;
    RequestParams params(_params);
    PropfindRequest* http_req = new PropfindRequest(*context, url, &tmp_err);

    // create a new connexion + parser for this opendir
    if(tmp_err == NULL){
        configure_req_for_listdir(http_req);
        ScopedPtr<DIR_handle>::type res(  new DIR_handle(http_req, new DavPropXMLParser()));
        time_t timestamp_timeout = time(NULL) + _timeout;

        http_req->setParameters(params);
        DavPropXMLParser* parser = res->parser;
        // setup the handle for simple listing only
        http_req->setRequestBody(body);


        if( (ret = http_req->beginRequest(&tmp_err)) == 0){ // start req


            if( ( ret = davixRequestToFileStatus(http_req,davix_scope_directory_listing_str(), &tmp_err)) == 0){

                    size_t prop_size = 0;
                    do{ // parse the begining of the request until the first property -> directory property
                       if( (s_resu = incremental_propfind_listdir_parsing(http_req, parser, this->_s_buff, scope, &tmp_err)) <0)
                           break;

                       prop_size = parser->getProperties().size();
                       if(s_resu < _s_buff && prop_size <1){ // verify request status : if req done + no data -> error
                           DavixError::setupError(&tmp_err, davix_scope_directory_listing_str(), StatusCode::WebDavPropertiesParsingError, "request answer incorrect, not a valid webdav request");
                           break;
                       }
                       if(timestamp_timeout < time(NULL)){
                           DavixError::setupError(&tmp_err, davix_scope_directory_listing_str(), StatusCode::OperationTimeout, "operation timeout triggered while directory listing");
                           break;
                       }

                    }while( prop_size < 1); // leave is end of req & no data

                    if(!tmp_err){
                        if( S_ISDIR(parser->getProperties().at(0).mode) == false){
                             DavixError::setupError(&tmp_err, davix_scope_directory_listing_str(), StatusCode::IsNotADirectory, url + " is not a collection or a directory, impossible to list");
                        }else{
                            parser->getProperties().pop_front(); // suppress the parent directory infos...
                            r = res.release(); // success : take ownership of the pointer
                        }
                    }
            }
        }
    }

    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return (DAVIX_DIR*) r;
}


DAVIX_DIR* DavPosix::opendir(const RequestParams* params, const std::string &url, DavixError** err){

    DAVIX_DEBUG(" -> davix_opendir");
    DAVIX_DIR* r = internal_opendirpp(params, "Core::opendir",simple_listing, url, err);

    DAVIX_DEBUG(" <- davix_opendir");
    return (DAVIX_DIR*) r;
}

DAVIX_DIR* DavPosix::opendirpp(const RequestParams* params, const std::string &url, DavixError** err){

    DAVIX_DEBUG(" -> davix_opendirpp");
    DAVIX_DIR* r = internal_opendirpp(params, "Core::opendir",stat_listing, url, err);

    DAVIX_DEBUG(" <- davix_opendirpp");
    return (DAVIX_DIR*) r;
}


struct dirent* DavPosix::readdir(DAVIX_DIR * d, DavixError** err){
    DIR_handle* handle = static_cast<DIR_handle*>(d);
    DavixError* tmp_err=NULL;
    DAVIX_DEBUG(" -> davix_readdir");

    if( d==NULL){
        DavixError::setupError(err, davix_scope_directory_listing_str(), StatusCode::InvalidFileHandle,  "Invalid file descriptor for DAVIX_DIR*");
        return NULL;
    }


    HttpRequest *req = handle->request; // setup env again
    DavPropXMLParser* parser = handle->parser;
    off_t read_offset = handle->dir_offset+1;
    size_t prop_size = parser->getProperties().size();
    ssize_t s_resu = _s_buff;

    while( prop_size == 0
          && s_resu > 0){ // request not complete and current data too smalls
        // continue the parsing until one more result
       if( (s_resu = incremental_propfind_listdir_parsing(req, parser, this->_s_buff, "Davix::readdir", &tmp_err)) <0)
           break;

       prop_size = parser->getProperties().size();
    }
    if(!tmp_err){
        if(prop_size == 0) // end of the request, end of the story
            return NULL;
        fill_dirent_from_filestat(handle->dir_info, parser->getProperties().front());
        handle->dir_offset = read_offset;
        parser->getProperties().pop_front(); // clean the current element
        DAVIX_DEBUG(" <- davix_readdir");
        return handle->dir_info;
    }


    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return NULL;
}

struct dirent* DavPosix::readdirpp(DAVIX_DIR * d, struct stat *st, DavixError** err){
    DAVIX_DEBUG(" -> davix_readdirpp");

    if( d==NULL){
        DavixError::setupError(err, davix_scope_directory_listing_str(), StatusCode::InvalidFileHandle,  "Invalid file descriptor for DAVIX_DIR*");
        return NULL;
    }

    DIR_handle* handle = static_cast<DIR_handle*>(d);
    struct dirent* ret = NULL;
    DavixError* tmp_err=NULL;

    HttpRequest* req = handle->request; // setup env again
    DavPropXMLParser* parser = handle->parser;
    off_t read_offset = handle->dir_offset+1;
    size_t prop_size = parser->getProperties().size();
    ssize_t s_resu = _s_buff;

    while( prop_size == 0
          && s_resu > 0){ // request not complete and current data too smalls
        // continue the parsing until one more result
       if( (s_resu = incremental_propfind_listdir_parsing(req, parser, this->_s_buff, "Davix::readdirpp", &tmp_err)) <0)
           break;

       prop_size = parser->getProperties().size();
    }

    if(!tmp_err){
        if(prop_size == 0){
            ret= NULL; // end of the request, end of the story
        }else{
            fill_dirent_from_filestat(handle->dir_info, parser->getProperties().front());
            handle->dir_offset = read_offset;
            fill_stat_from_fileproperties(st, parser->getProperties().front());
            parser->getProperties().pop_front(); // clean the current element
            ret= handle->dir_info;
        }
        DAVIX_DEBUG(" <- davix_readdirpp");
    }

    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}


int DavPosix::closedirpp(DAVIX_DIR * d, DavixError** err){
    int ret =-1;
    if( d==NULL){
        Davix::DavixError::setupError(err,davix_scope_directory_listing_str(),Davix::StatusCode::InvalidFileHandle, "Invalid file descriptor for DAVIX_DIR*");
   }else{
        delete (static_cast<DIR_handle*>(d));
        ret =0;
    }
    return ret;
}

int DavPosix::closedir(DAVIX_DIR * d, DavixError** err){
    return closedirpp(d,err);
}

void DavPosix::fadvise(DAVIX_FD *fd, dav_off_t offset, dav_size_t len, advise_t advise){
    if( fd==NULL)
        return;
    fd->io_handler->prefetchInfo(offset, len, advise);
}


////////////////////////////////////////////////////
//////////////////// Davix POSIX meta ops
/////////////////////////////////////////////////////


int DavPosix::mkdir(const RequestParams * _params, const std::string &url, mode_t right, DavixError** err){
    DAVIX_DEBUG(" -> davix_mkdir");
    RequestParams params(_params);
    int ret  = Meta::makeCollection(*context, url, params, err);
    DAVIX_DEBUG(" davix_mkdir <-");
    return ret;
}


int DavPosix::stat(const RequestParams * params, const std::string & url, struct stat* st, DavixError** err){
    DAVIX_DEBUG(" -> davix_stat");
    DavixError* tmp_err=NULL;

    int ret = Meta::posixStat(*context, Uri(url), params, st, NULL, &tmp_err);

    DAVIX_DEBUG(" davix_stat <-");
    if(tmp_err)
        DavixError::propagatePrefixedError(err, tmp_err, "stat ops : ");
    return ret;

}

int davix_remove_posix(DavPosix & p, Context* c, const RequestParams * params, const std::string & url, bool directory, DavixError** err){
    DavixError* tmp_err = NULL;
    int ret = -1;
    Uri uri(url);
    WebdavQuery query(*c);

    if(params && params->getProtocol() == RequestProtocol::Http){ // pure protocol http : ignore posix semantic, execute a simple delete
        ret = query.davDelete(params, uri, &tmp_err);
    }else{ // full posix semantic support
        struct stat st;
        ret = p.stat(params, url, &st, &tmp_err);
        if( ret ==0){
            if( S_ISDIR(st.st_mode)){ // directory : impossible to delete if not empty
                if(directory == true){
                    // ignore non empty dir check for now
                    ret = query.davDelete(params, uri, &tmp_err);
                }else{
                    ret = -1;
                    std::ostringstream ss;
                    ss << " " << url << " is not a directory, impossible to unlink"<< std::endl;
                    DavixError::setupError(&tmp_err, davix_scope_davOps_str(), StatusCode::IsADirectory, ss.str());
                }
            }else{ // file, rock & roll
                if(directory == false){
                    ret = query.davDelete(params, uri, &tmp_err);
                }else{
                    ret = -1;
                    std::ostringstream ss;
                    ss << " " << url << " is not a directory, impossible to rmdir"<< std::endl;
                    DavixError::setupError(&tmp_err, davix_scope_davOps_str(), StatusCode::IsNotADirectory, ss.str());
                }
            }
        }
    }
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;

}


int DavPosix::unlink(const RequestParams * params, const std::string &url, DavixError** err){
    DAVIX_DEBUG(" -> davix_unlink");
    int ret=-1;
    DavixError* tmp_err=NULL;

    ret = davix_remove_posix(*this, context, params, url, false, &tmp_err);
    DAVIX_DEBUG(" davix_unlink <-");
    if(tmp_err)
        DavixError::propagatePrefixedError(err, tmp_err, "DavPosix::unlink ");
    return ret;
}


int DavPosix::rmdir(const RequestParams * params, const std::string &url, DavixError** err){
    DAVIX_DEBUG(" -> davix_rmdir");
    int ret=-1;
    DavixError* tmp_err=NULL;

    ret = davix_remove_posix(*this, context, params, url, true, &tmp_err);
    DAVIX_DEBUG(" davix_rmdir <-");
    if(tmp_err)
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
    DAVIX_DEBUG(" -> davix_open");
    DavixError* tmp_err=NULL;
    Davix_fd* fd = NULL;
    Uri uri(url);

    if(uri.getStatus() == StatusCode::OK){
        fd = new Davix_fd( new HttpIOBuffer(*context, uri, _params));
        fd->io_handler->open( flags, &tmp_err);
    }else{
        DavixError::setupError(&tmp_err, davix_scope_http_request(), uri.getStatus(), " Uri invalid in Davix::Open");
    }

    if(tmp_err){
        DavixError::propagateError(err, tmp_err);
        delete fd;
        fd= NULL;
    }
    DAVIX_DEBUG(" davix_open <-");
    return fd;
}


ssize_t DavPosix::read(DAVIX_FD* fd, void* buf, size_t count, Davix::DavixError** err){
    DAVIX_DEBUG(" -> davix_read");
    ssize_t ret =-1;
    DavixError* tmp_err=NULL;

    if( davix_check_rw_fd(fd, &tmp_err) ==0){
        ret = (ssize_t) fd->io_handler->read(buf, (dav_size_t) count, &tmp_err);
    }


    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    DAVIX_DEBUG(" davix_read <-");
    return ret;
}


ssize_t DavPosix::pread(DAVIX_FD* fd, void* buf, size_t count, off_t offset, DavixError** err){
    DAVIX_DEBUG(" -> davix_pread");
    ssize_t ret =-1;
    DavixError* tmp_err=NULL;

    if( davix_check_rw_fd(fd, &tmp_err) ==0){
        ret = (ssize_t) fd->io_handler->pread(buf, (dav_size_t)  count, (dav_off_t) offset, &tmp_err);
    }


    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    DAVIX_DEBUG(" davix_pread <-");
    return ret;
}

dav_ssize_t DavPosix::preadVec(DAVIX_FD* fd, const DavIOVecInput * input_vec,
                      DavIOVecOuput * output_vec,
                      dav_size_t count_vec, DavixError** err){
    DAVIX_DEBUG(" -> davix_pread_vec");
    ssize_t ret =-1;
    DavixError* tmp_err=NULL;

    if( davix_check_rw_fd(fd, &tmp_err) ==0){
        ret = fd->io_handler->preadVec(input_vec, output_vec, count_vec, &tmp_err);
    }


    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    DAVIX_DEBUG(" davix_pread_vec <-");
    return ret;
}

ssize_t DavPosix::write(DAVIX_FD* fd, const void* buf, size_t count, Davix::DavixError** err){
    DAVIX_DEBUG(" -> davix_write");
    ssize_t ret =-1;
    DavixError* tmp_err=NULL;

    if( davix_check_rw_fd(fd, &tmp_err) ==0){
        ret = (ssize_t) fd->io_handler->write(buf, (dav_size_t) count, &tmp_err);
    }


    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    DAVIX_DEBUG(" davix_write <-");
    return ret;
}



off_t DavPosix::lseek(DAVIX_FD* fd, off_t offset, int flags, Davix::DavixError** err){
    DAVIX_DEBUG(" -> davix_lseek");
    ssize_t ret =-1;
    DavixError* tmp_err=NULL;

    if( davix_check_rw_fd(fd, &tmp_err) ==0){
        ret = (off_t) fd->io_handler->lseek( (dav_off_t) offset, flags, &tmp_err);
    }


    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    DAVIX_DEBUG(" davix_lseek <-");
    return ret;
}


int DavPosix::close(DAVIX_FD* fd, Davix::DavixError** err){
    if(fd){
        delete fd;
    }
    return 0;
}


} // namespace Davix
