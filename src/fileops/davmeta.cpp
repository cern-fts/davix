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

#include <davix_internal.hpp>
#include "davmeta.hpp"

#include <xml/davpropxmlparser.hpp>
#include <utils/davix_logger_internal.hpp>
#include <request/httprequest.hpp>
#include <fileops/fileutils.hpp>
#include <utils/davix_utils_internal.hpp>
#include <string_utils/stringutils.hpp>
#include <xml/metalinkparser.hpp>
#include <base64/base64.hpp>


using namespace StrUtil;

namespace Davix{





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


int dav_stat_mapper_webdav(Context &context, const RequestParams* params, const Uri & url, struct StatInfo& st_info){
    int ret =-1;

    DavPropXMLParser parser;
    DavixError * tmp_err=NULL;
    HttpRequest req(context, url, &tmp_err);
    if( tmp_err == NULL){
        req.setParameters(params);

        TRY_DAVIX{
            const char * res = req_webdav_propfind(&req, &tmp_err);
            if(!tmp_err){
               parser.parseChunk((const char*) res, strlen(res));

                std::deque<FileProperties> & props = parser.getProperties();
                if( props.size() < 1){
                    throw DavixException(davix_scope_stat_str(), Davix::StatusCode::WebDavPropertiesParsingError, "Parsing Error : properties number < 1");
                }else{
                    fill_fileinfo_from_fileproperties( props.front(), st_info);
                    ret =0;
                }
            }
        }CATCH_DAVIX(&tmp_err)
        if(tmp_err != NULL)
            ret = -1;
    }
    checkDavixError(&tmp_err);
    return ret;
}


int dav_stat_mapper_http(Context& context, const RequestParams* params, const Uri & uri, struct StatInfo& st_info){
    int ret = -1;
    DavixError * tmp_err=NULL;
    HeadRequest req(context, uri, &tmp_err);

    if( tmp_err == NULL){
        req.setParameters(params);
        req.executeRequest(&tmp_err);

        if(!tmp_err){
            if(httpcodeIsValid(req.getRequestCode()) ){
                memset(&st_info, 0, sizeof(struct StatInfo));
                const dav_ssize_t s = req.getAnswerSize();
                st_info.size = std::max<dav_ssize_t>(0,s);
                st_info.mode = 0755 | S_IFREG;
                ret = 0;
            }else{
                httpcodeToDavixError(req.getRequestCode(), davix_scope_http_request(), uri.getString() , &tmp_err);
                ret = -1;
            }
        }
    }
    checkDavixError(&tmp_err);
    return ret;
}


dav_ssize_t getStatInfo(Context & c, const Uri & url, const RequestParams * params,
                      struct StatInfo& st_info){
    RequestParams _params(params);
    int ret =-1;
    configureRequestParamsProto(url, _params);

    switch(_params.getProtocol()){
         case RequestProtocol::Webdav:
            ret = dav_stat_mapper_webdav(c, &_params, url, st_info);
            break;
        default:
            ret = dav_stat_mapper_http(c, &_params, url, st_info);
            break;

    }
    DAVIX_DEBUG(" davix_stat <-");
    return ret;
}

void parse_creation_deletion_result(int code, const Uri & u, const std::string & msg){
    switch(code){
    case 200:
    case 201:
    case 202:
    case 204:{
            return;
    }
    case 207:{
        // parse webdav
        DavPropXMLParser parser;
        parser.parseChunk(msg);
        if( parser.getProperties().size() > 0
           && httpcodeIsValid(parser.getProperties().at(0).req_status)){
            return;
        }
        break;
    }
    }
    httpcodeToDavixException(code, davix_scope_stat_str(), msg);
}


int internal_deleteResource(Context & c, const Uri & url, const RequestParams & params, DavixError** err){
    DavixError* tmp_err=NULL;
    int ret=-1;
    RequestParams _params(params);
    configureRequestParamsProto(url, _params);

    DeleteRequest req(c,url, err);
    req.setParameters(_params);
    if(!tmp_err){
         if( ( ret=req.executeRequest(&tmp_err)) == 0){
                parse_creation_deletion_result(req.getRequestCode(), url, req.getAnswerContent());
         }
    }


    DavixError::propagateError(err, tmp_err);
    return ret;
}


int internal_makeCollection(Context & c, const Uri & url, const RequestParams & params, DavixError** err){
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
                parse_creation_deletion_result(req.getRequestCode(), url, req.getAnswerContent());
        }

        DAVIX_DEBUG(" makeCollection <-");
    }

    DavixError::propagateError(err, tmp_err);
    return ret;
}


int internal_checksum(Context & c, const Uri & url, const RequestParams *params, std::string & checksm, const std::string & chk_algo){
    DAVIX_DEBUG(" -> checksum");
    int ret=-1;
    DavixError* tmp_err=NULL;
    RequestParams _params(params);
    configureRequestParamsProto(url, _params);

    HeadRequest req(c, url, &tmp_err);

    if(tmp_err == NULL){
        // add Digest file, support for other digest, extended format
        req.addHeaderField("Want-Digest", chk_algo);
        req.setParameters(params);
        if( (ret = req.executeRequest(&tmp_err)) == 0
            && (ret = davixRequestToFileStatus(&req, davix_scope_mkdir_str(), &tmp_err)) >=0){

            // try simple MD5 ( standard )
            if(compare_ncase(chk_algo, "MD5") == 0){
                std::string  chk;
                if(req.getAnswerHeader("Content-MD5", chk) == true){
                    DAVIX_TRACE("Extract MD5 checksum in base64 %s", chk.c_str());
                    chk= Base64::base64_decode(chk);
                    std::swap(checksm, chk);
                }
            }

            // fallback on extension for checksum
            std::string digest;
            req.getAnswerHeader("Digest", digest);
            if (digest.empty())
                throw DavixException(davix_scope_meta(), StatusCode::OperationNonSupported, "Checksum calculation not supported by server");

            size_t valueOffset = digest.find('=');
            if (valueOffset == std::string::npos
                    || compare_ncase(digest,0, valueOffset, chk_algo.c_str()) !=0)
                throw DavixException(davix_scope_meta(), StatusCode::InvalidServerResponse, "Invalid server checksum answer");

            digest.erase(digest.begin(), digest.begin()+valueOffset+1);
            std::swap(checksm, digest);

            DAVIX_DEBUG(" checksum <-");
            return 0;
        }
    }
    throw DavixException(&tmp_err);
}


HttpMetaOps::HttpMetaOps(): HttpIOChain(){}

HttpMetaOps::~HttpMetaOps(){}


void HttpMetaOps::checksum(IOChainContext & iocontext, std::string &checksm, const std::string &chk_algo){
    internal_checksum(iocontext._context, iocontext._uri, iocontext._reqparams, checksm, chk_algo);
}

void HttpMetaOps::makeCollection(IOChainContext & iocontext){
    DavixError* tmp_err=NULL;
    internal_makeCollection(iocontext._context, iocontext._uri, iocontext._reqparams, &tmp_err);
    checkDavixError(&tmp_err);
}

void HttpMetaOps::deleteResource(IOChainContext & iocontext){
    DavixError* tmp_err=NULL;
    internal_deleteResource(iocontext._context, iocontext._uri, iocontext._reqparams, &tmp_err);
    checkDavixError(&tmp_err);
}

StatInfo & HttpMetaOps::statInfo(IOChainContext & iocontext, StatInfo &st_info){
    DavixError* tmp_err=NULL;
    struct stat st;
    memset(&st, 0, sizeof(struct stat));
    getStatInfo(iocontext._context, iocontext._uri, iocontext._reqparams, st_info);
    checkDavixError(&tmp_err);
    return st_info;
}



} // Davix
