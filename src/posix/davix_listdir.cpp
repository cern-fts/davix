#include <sys/stat.h>
#include <dirent.h>
#include <ostream>
#include <sstream>

#include "davix_listdir.hpp"
#include <posix/davposix.hpp>
#include <contextinternal.h>


#include <httprequest.hpp>
#include <posix/davix_stat.hpp>
#include <status/davixstatusrequest.hpp>
#include <fileops/fileutils.hpp>

static const std::string simple_listing("<propfind xmlns=\"DAV:\"><prop></prop></propfind>");

static const std::string stat_listing("<propfind xmlns=\"DAV:\"><prop><getlastmodified/><creationdate/><getcontentlength/><resourcetype><collection/></resourcetype><mode/></prop></propfind>");


struct Davix_dir_handle{

    Davix_dir_handle(Davix::HttpRequest* _req, Davix::DavPropXMLParser * _parser){
        request = _req;
        parser = _parser;
        size_t s_dirent = sizeof(struct dirent) + NAME_MAX +1;
        dir_info = (struct dirent*) calloc(1, s_dirent);
    }

   ~Davix_dir_handle(){
    delete request;
    delete parser;
    free(dir_info);
    }

   Davix::HttpRequest* request;
   Davix::DavPropXMLParser * parser;
   struct dirent* dir_info;

};


namespace Davix{

typedef Davix_dir_handle DIR_handle;

static void fill_dirent_from_filestat(struct dirent * d, const FileProperties & f, const off_t offset){
    d->d_off = offset;
    g_strlcpy(d->d_name, f.filename.c_str(), NAME_MAX);
}

int incremental_propfind_listdir_parsing(HttpRequest* req, DavPropXMLParser * parser, size_t s_buff, const char* scope, DavixError** err){
  //  std::cout << "time 1 pre-fecth" << time(NULL) << std::endl;
    DavixError* tmp_err=NULL;

    char buffer[s_buff+1];
    const ssize_t ret_s_buff= req->read_block(buffer, s_buff, &tmp_err);
    if(ret_s_buff >= 0){
        buffer[ret_s_buff]= '\0';
        davix_log_debug("chunk parse : result content : %s", buffer);
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
    req->setRequestMethod("PROPFIND");
}

DAVIX_DIR* DavPosix::internal_opendirpp(const RequestParams* _params, const char * scope, const std::string & body, const std::string & url, DavixError** err ){
    ssize_t s_resu;
    DAVIX_DIR* r = NULL;
    int ret =-1;
    DavixError* tmp_err=NULL;
    RequestParams params(_params);


    // create a new connexion + parser for this opendir
    HttpRequest* http_req = static_cast<HttpRequest*>(context->_intern->getSessionFactory()->create_request(url, &tmp_err));
    if(http_req){
        configure_req_for_listdir(http_req);
        std::auto_ptr<DIR_handle> res(  new DIR_handle(http_req, new DavPropXMLParser()));
        time_t timestamp_timeout = time(NULL) + _timeout;

        http_req->set_parameters(params);
        DavPropXMLParser* parser = res->parser;
        // setup the handle for simple listing only
        http_req->add_full_request_content(body);


        if( (ret = http_req->execute_block(&tmp_err)) == 0){ // start req


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
                        parser->getProperties().pop_front(); // suppress the parent directory infos...
                        r = res.release(); // success : take ownership of the pointer
                    }
            }
        }
    }

    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return (DAVIX_DIR*) r;
}


DAVIX_DIR* DavPosix::opendir(const RequestParams* params, const std::string &url, DavixError** err){

    davix_log_debug(" -> davix_opendir");
    DAVIX_DIR* r = internal_opendirpp(params, "Core::opendir",simple_listing, url, err);

    davix_log_debug(" <- davix_opendir");
    return (DAVIX_DIR*) r;
}

DAVIX_DIR* DavPosix::opendirpp(const RequestParams* params, const std::string &url, DavixError** err){

    davix_log_debug(" -> davix_opendirpp");
    DAVIX_DIR* r = internal_opendirpp(params, "Core::opendir",stat_listing, url, err);

    davix_log_debug(" <- davix_opendirpp");
    return (DAVIX_DIR*) r;
}


struct dirent* DavPosix::readdir(DAVIX_DIR * d, DavixError** err){
    DIR_handle* handle = static_cast<DIR_handle*>(d);
    DavixError* tmp_err=NULL;
    davix_log_debug(" -> davix_readdir");

    if( d==NULL){
        DavixError::setupError(err, davix_scope_directory_listing_str(), StatusCode::invalidFileHandle,  "Invalid file descriptor for DAVIX_DIR*");
        return NULL;
    }


    HttpRequest *req = handle->request; // setup env again
    DavPropXMLParser* parser = handle->parser;
    off_t read_offset = handle->dir_info->d_off+1;
    size_t prop_size = parser->getProperties().size();
    size_t s_resu = _s_buff;

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
        fill_dirent_from_filestat(handle->dir_info, parser->getProperties().front(), read_offset);
        parser->getProperties().pop_front(); // clean the current element
        davix_log_debug(" <- davix_readdir");
        return handle->dir_info;
    }


    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return NULL;
}

struct dirent* DavPosix::readdirpp(DAVIX_DIR * d, struct stat *st, DavixError** err){
    davix_log_debug(" -> davix_readdirpp");

    if( d==NULL){
        DavixError::setupError(err, davix_scope_directory_listing_str(), StatusCode::invalidFileHandle,  "Invalid file descriptor for DAVIX_DIR*");
        return NULL;
    }

    DIR_handle* handle = static_cast<DIR_handle*>(d);
    struct dirent* ret = NULL;
    DavixError* tmp_err=NULL;

    HttpRequest* req = handle->request; // setup env again
    DavPropXMLParser* parser = handle->parser;
    off_t read_offset = handle->dir_info->d_off+1;
    size_t prop_size = parser->getProperties().size();
    size_t s_resu = _s_buff;

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
            fill_dirent_from_filestat(handle->dir_info, parser->getProperties().front(), read_offset);
            fill_stat_from_fileproperties(st, parser->getProperties().front());
            parser->getProperties().pop_front(); // clean the current element
            ret= handle->dir_info;
        }
        davix_log_debug(" <- davix_readdirpp");
    }

    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}


int DavPosix::closedirpp(DAVIX_DIR * d, DavixError** err){
    int ret =-1;
    if( d==NULL){
        Davix::DavixError::setupError(err,davix_scope_directory_listing_str(),Davix::StatusCode::invalidFileHandle, "Invalid file descriptor for DAVIX_DIR*");
   }else{
        delete (static_cast<DIR_handle*>(d));
        ret =0;
    }
    return ret;
}

int DavPosix::closedir(DAVIX_DIR * d, DavixError** err){
    return closedirpp(d,err);
}

} // namespace Davix












DAVIX_C_DECL_BEGIN

DAVIX_DIR* davix_posix_opendir(davix_sess_t sess, davix_params_t _params, const char* url,  davix_error_t* err){
    g_return_val_if_fail(sess != NULL, NULL);
    DAVIX_DIR* ret = NULL;


    Davix::DavPosix p((Davix::Context*)(sess));
    Davix::RequestParams * params = (Davix::RequestParams*) (_params);
    ret= p.opendir(params,url, (Davix::DavixError**) err);

    return ret;
}



int davix_posix_closedir(davix_sess_t sess, DAVIX_DIR* d, davix_error_t* err){
    g_return_val_if_fail(sess != NULL,-1);

    Davix::DavPosix p((Davix::Context*)(sess));


    return p.closedir(d,(Davix::DavixError**) err);

}


struct dirent* davix_posix_readdir(davix_sess_t sess, DAVIX_DIR* d, davix_error_t* err){
    g_return_val_if_fail(sess != NULL,NULL);

    struct dirent* ret = NULL;
    Davix::DavPosix p((Davix::Context*)(sess));

    if(d){
        ret= p.readdir(d, (Davix::DavixError**) err);
    }

    return ret;
}


DAVIX_C_DECL_END



