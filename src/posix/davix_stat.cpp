#include "davix_stat.hpp"

#include <ostream>
#include <sstream>
#include <string>
#include <cstring>
#include <status/davixstatusrequest.hpp>
#include <fileops/fileutils.hpp>

#include <contextinternal.h>

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
        req->set_parameters(params);

        const std::vector<char> & res = req_webdav_propfind(req.get(), &tmp_err);
        if(!tmp_err){
            if( (ret = parser.parseChuck((const char*) & res.at(0), res.size() -1) ) < 0){
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
        req->set_parameters(params);
        req->setRequestMethod("HEAD");
        req->executeRequest(&tmp_err);

        if(!tmp_err){
            if(httpcodeIsValid(req->getRequestCode()) ){
                memset(st, 0, sizeof(struct stat));
                std::string content_length;
                //req->getAns

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
    davix_log_debug(" -> davix_stat");
    RequestParams params(_params);
    DavixError* tmp_err=NULL;

    int ret =-1;

    switch(params.getProtocol()){
         case DAVIX_PROTOCOL_HTTP:
            ret = -1;
            break;
        default:
            ret = dav_stat_mapper_webdav(context, &params, url, st, &tmp_err);
            break;
    }
    davix_log_debug(" davix_stat <-");
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;

}

/**
  execute a propfind/stat request on a given HTTP request handle
  return a vector with the content of the request if success
*/
const std::vector<char> & req_webdav_propfind(HttpRequest* req, DavixError** err){
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
    g_return_val_if_fail(sess != NULL,-1);

    Davix::DavPosix p((Davix::Context*)(sess));
    Davix::RequestParams * params = (Davix::RequestParams*) (_params);

    return p.stat(params,url, st, (Davix::DavixError**) err);

}

DAVIX_C_DECL_END
