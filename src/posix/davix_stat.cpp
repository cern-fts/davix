#include <config.h>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <logger/davix_logger_internal.h>
#include <status/davixstatusrequest.hpp>
#include <fileops/fileutils.hpp>


#include "davix_stat.hpp"




namespace Davix{



void fill_stat_from_fileproperties(struct stat* st, const  FileProperties & prop){
    memset(st, 0, sizeof(struct stat));
    st->st_mtime = prop.mtime;
    st->st_atime = prop.atime;
    st->st_ctime = prop.ctime;
    st->st_size = prop.size;
    st->st_mode = prop.mode;
}


int dav_stat_mapper_webdav(Context* context, const RequestParams & params, const std::string & url, struct stat* st, DavixError** err){
    int ret =-1;

    DavPropXMLParser parser;
    std::auto_ptr<HttpRequest> req( context->createRequest(url, err));
    DavixError * tmp_err=NULL;
    if( req.get() != NULL){
        req->setParameters(params);

        const char * res = req_webdav_propfind(req.get(), &tmp_err);
        if(!tmp_err){
            if( (ret = parser.parseChuck((const char*) res, strlen(res)) ) < 0){
                DavixError::propagateError(err, parser.getLastErr());
                return -1;
            }

            std::deque<FileProperties> & props = parser.getProperties();
            if( props.size() < 1){
                DavixError::setupError(&tmp_err, davix_scope_stat_str(), Davix::StatusCode::WebDavPropertiesParsingError, "Parsing Error : properties number < 1");
                ret =-1;
            }else{
                fill_stat_from_fileproperties(st, props.front());
                ret =0;
        }
        }
    }
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}


int dav_stat_mapper_http(Context* context, const RequestParams & params, const std::string & url, struct stat* st, DavixError** err){
    int ret = -1;
    std::auto_ptr<HttpRequest> req( context->createRequest(url, err));
    DavixError * tmp_err=NULL;
    if( req.get() != NULL){
        req->setParameters(params);
        req->setRequestMethod("HEAD");
        req->executeRequest(&tmp_err);

        if(!tmp_err){
            if(httpcodeIsValid(req->getRequestCode()) ){
                memset(st, 0, sizeof(struct stat));
                std::string content_length;
                st->st_mode = 0755;
                if( req->getAnswerHeader("Content-Length", content_length) ){
                     unsigned long l = strtoul(content_length.c_str(), NULL,10);
                     if(l != ULONG_MAX){
                         st->st_size = l;
                         ret = 0;
                     }
                }else{ // no data
                    st->st_size =0;
                    ret =0;
                }
                if(ret != 0){
                    DavixError::setupError(&tmp_err, davix_scope_stat_str(), StatusCode::WebDavPropertiesParsingError, " Invalid HEAD content");
                }
            }else{
                httpcodeToDavixCode(req->getRequestCode(), davix_scope_stat_str(), url , &tmp_err);
                ret = -1;
            }
        }
    }
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}


int DavPosix::stat(const RequestParams * _params, const std::string & url, struct stat* st, DavixError** err){
    DAVIX_DEBUG(" -> davix_stat");
    RequestParams params(_params);
    DavixError* tmp_err=NULL;

    int ret =-1;

    switch(params.getProtocol()){
         case DAVIX_PROTOCOL_HTTP:
            ret = dav_stat_mapper_http(context, &params, url, st, &tmp_err);
            break;
        default:
            ret = dav_stat_mapper_webdav(context, &params, url, st, &tmp_err);
            break;
    }
    DAVIX_DEBUG(" davix_stat <-");
    if(tmp_err)
        DavixError::propagatePrefixedError(err, tmp_err, "stat ops : ");
    return ret;

}

/**
  execute a propfind/stat request on a given HTTP request handle
  return a vector with the content of the request if success
*/
const char* req_webdav_propfind(HttpRequest* req, DavixError** err){
    DavixError* tmp_err=NULL;
    int ret =-1;

    req->addHeaderField("Depth","0");
    req->setRequestMethod("PROPFIND");
    if( (ret = req->executeRequest(&tmp_err)) ==0){
        ret = davixRequestToFileStatus(req, davix_scope_stat_str(), &tmp_err);
    }

    if(ret != 0)
        DavixError::propagateError(err, tmp_err);

    return req->getAnswerContent();
}


}


DAVIX_C_DECL_BEGIN

int davix_posix_stat(davix_sess_t sess, davix_params_t _params, const char* url, struct stat * st, davix_error_t* err){
    davix_return_val_if_fail(sess != NULL,-1);

    Davix::DavPosix p((Davix::Context*)(sess));
    Davix::RequestParams * params = (Davix::RequestParams*) (_params);

    return p.stat(params,url, st, (Davix::DavixError**) err);

}

DAVIX_C_DECL_END
