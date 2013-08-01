#include "davmeta.hpp"
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <deque>
#include <xml/davpropxmlparser.hpp>
#include <logger/davix_logger_internal.h>
#include <request/httprequest.hpp>
#include <fileops/fileutils.hpp>
#include <fileops/davops.hpp>
#include <utils/davix_utils_internal.hpp>


namespace Davix{

namespace Meta{


static const std::string propfind_request_replicas("<D:propfind xmlns:D=\"DAV:\" xmlns:L=\"LCGDM:\">"
                                                   "<D:prop><L:replicas/></D:prop>"
                                                   "</D:propfind>");


// get all reps from webdav queries
dav_ssize_t getAllReplicas(Context & c, const Uri & uri,
                              const RequestParams & params, ReplicaVec & vec, DavixError** err){
    dav_ssize_t ret =-1;
    DavixError* tmp_err=NULL;
    PropfindRequest req(c, uri, &tmp_err);
    if(tmp_err == NULL){
        req.setParameters(params);
        req.setRequestBody(propfind_request_replicas);
        if( req.executeRequest(&tmp_err) == 0){

        }
    }

    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
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


int dav_stat_mapper_webdav(Context &context, const RequestParams* params, const Uri & url, struct stat* st, HttpCacheToken** token_ptr,
                           DavixError** err){
    int ret =-1;

    DavPropXMLParser parser;
    DavixError * tmp_err=NULL;
    HttpRequest req(context, url, &tmp_err);
    if( tmp_err == NULL){
        req.setParameters(params);

        const char * res = req_webdav_propfind(&req, &tmp_err);
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
                if(token_ptr)
                    *token_ptr = req.extractCacheToken();
        }
        }
    }
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}


int dav_stat_mapper_http(Context& context, const RequestParams* params, const Uri & uri, struct stat* st, HttpCacheToken** token_ptr,
                         DavixError** err){
    int ret = -1;
    DavixError * tmp_err=NULL;
    HeadRequest req(context, uri, &tmp_err);

    if( tmp_err == NULL){
        req.setParameters(params);
        req.executeRequest(&tmp_err);

        if(!tmp_err){
            if(httpcodeIsValid(req.getRequestCode()) ){
                memset(st, 0, sizeof(struct stat));
                const dav_ssize_t s = req.getAnswerSize();
                st->st_size = (size_t) (s <0)?0:s;
                st->st_mode = 0755;
                ret = 0;
                if(token_ptr)
                    *token_ptr = req.extractCacheToken();
            }else{
                httpcodeToDavixCode(req.getRequestCode(), davix_scope_http_request(), uri.getString() , &tmp_err);
                ret = -1;
            }
        }
    }
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}


dav_ssize_t posixStat(Context & c, const Uri & url, const RequestParams * params,
                      struct stat* st, HttpCacheToken** token_ptr,
                      DavixError** err){
    RequestParams _params(params);
    DavixError* tmp_err=NULL;
    int ret =-1;
    configureRequestParamsProto(url, _params);

    switch(_params.getProtocol()){
         case RequestProtocol::Webdav:
            ret = dav_stat_mapper_webdav(c, &_params, url, st, token_ptr, &tmp_err);
            break;
        default:
            ret = dav_stat_mapper_http(c, &_params, url, st, token_ptr, &tmp_err);
            break;

    }
    DAVIX_DEBUG(" davix_stat <-");
    if(tmp_err)
        DavixError::propagatePrefixedError(err, tmp_err, "stat ops : ");
    return ret;
}

int deleteResource(Context & c, const Uri & url, const RequestParams & params, DavixError** err){
    DavixError* tmp_err=NULL;
    int ret=-1;
    RequestParams _params(params);
    configureRequestParamsProto(url, _params);

    DeleteRequest req(c,url, err);
    req.setParameters(_params);
    if(!tmp_err){
        ret=req.executeRequest(&tmp_err);
        if(!tmp_err && httpcodeIsValid(req.getRequestCode()) == false){
                httpcodeToDavixCode(req.getRequestCode(), davix_scope_stat_str(), url.getString() , &tmp_err);
                ret = -1;
         }
    }


    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}


int makeCollection(Context & c, const Uri & url, const RequestParams & params, DavixError** err){
    DAVIX_DEBUG(" -> makeCollection");
    int ret=-1;
    DavixError* tmp_err=NULL;
    RequestParams _params(params);
    configureRequestParamsProto(url, _params);

    HttpRequest req(c, url, &tmp_err);

    if(tmp_err == NULL){
        req.setParameters(params);
        req.setRequestMethod("MKCOL");
        if( (ret = req.executeRequest(&tmp_err)) == 0){
            ret = davixRequestToFileStatus(&req, davix_scope_mkdir_str(), &tmp_err);
        }

        DAVIX_DEBUG(" makeCollection <-");
    }
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}

} // Meta

} // Davix
