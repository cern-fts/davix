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
#include <xml/metalinkparser.hpp>
#include <xml/s3propparser.hpp>

#include <utils/davix_logger_internal.hpp>
#include <utils/davix_utils_internal.hpp>

#include <request/httprequest.hpp>
#include <fileops/fileutils.hpp>
#include <string_utils/stringutils.hpp>
#include <base64/base64.hpp>
#include <neon/neonrequest.hpp>


using namespace StrUtil;

namespace Davix{

static const std::string simple_listing("<propfind xmlns=\"DAV:\"><prop><resourcetype><collection/></resourcetype></prop></propfind>");

static const std::string stat_listing("<?xml version=\"1.0\" encoding=\"utf-8\" ?><D:propfind xmlns:D=\"DAV:\" xmlns:L=\"LCGDM:\"><D:prop>"
                                      "<D:displayname/><D:getlastmodified/><D:creationdate/><D:getcontentlength/>"
                                      "<D:resourcetype><D:collection/></D:resourcetype><L:mode/>"
                                      "</D:prop>"
                                      "</D:propfind>");



class DirHandle{
public:
    DirHandle(HttpRequest* req, XMLPropParser * p): request(req), parser(p){}

    Ptr::Scoped<HttpRequest> request;
    Ptr::Scoped<Davix::XMLPropParser> parser;
private:
    DirHandle(const DirHandle &);
};

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
                    st_info = props.front().info;
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


dav_ssize_t getStatInfo(Context & c, const Uri & url, const RequestParams * p,
                      struct StatInfo& st_info){
    RequestParams params(p);
    configureRequestParamsProto(url, params);
    int ret =-1;

    switch(params.getProtocol()){
         case RequestProtocol::Webdav:
            ret = dav_stat_mapper_webdav(c, &params, url, st_info);
            break;
        default:
            ret = dav_stat_mapper_http(c, &params, url, st_info);
            break;

    }
    DAVIX_DEBUG(" davix_stat <-");
    return ret;
}

void parse_creation_deletion_result(int code, const Uri & u, const std::string & scope, const std::string & msg){
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
            if( parser.getProperties().size() > 0){
               const int sub_code = parser.getProperties().at(0).req_status;
               if(httpcodeIsValid(sub_code) == false){
                   httpcodeToDavixException(sub_code, scope);
               }
               return;
            }
            // if no properties, properties were filtered because invalid
            httpcodeToDavixException(404, scope);
            break;
        }
    }
    std::ostringstream ss;
    ss << " in url " << u.getString();
    httpcodeToDavixException(code, scope, ss.str());
}


int internal_delete_resource(Context & c, const Uri & url, const RequestParams & params){
    DavixError* tmp_err=NULL;
    int ret=-1;
    RequestParams _params(params);

    DeleteRequest req(c,url, &tmp_err);
    req.setParameters(_params);
    if(!tmp_err){
         if( ( ret=req.executeRequest(&tmp_err)) == 0){
                parse_creation_deletion_result(req.getRequestCode(), url, davix_scope_rm_str(), req.getAnswerContent());
         }
    }


    checkDavixError(&tmp_err);
    return ret;
}


int internal_make_collection(Context & c, const Uri & url, const RequestParams & params){
    DAVIX_DEBUG(" -> makeCollection");
    int ret=-1;
    DavixError* tmp_err=NULL;
    RequestParams _params(params);

    HttpRequest req(c, url, &tmp_err);

    if(tmp_err == NULL){
        req.setParameters(params);
        req.setRequestMethod("MKCOL");
        if( (ret = req.executeRequest(&tmp_err)) == 0){
                parse_creation_deletion_result(req.getRequestCode(),  url, davix_scope_mkdir_str(), req.getAnswerContent());
        }

    }

    DAVIX_DEBUG(" makeCollection <-");
    checkDavixError(&tmp_err);
    return ret;
}


int internal_move(Context & c, const Uri & url, const RequestParams & params, const std::string & target_url){
    DAVIX_DEBUG(" -> move");
    int ret=-1;
    DavixError* tmp_err=NULL;
    RequestParams _params(params);

    HttpRequest req(c, url, &tmp_err);

    if(tmp_err == NULL){
        req.setParameters(params);
        req.setRequestMethod("MOVE");
        req.addHeaderField("Destination", target_url);

        if( (ret = req.executeRequest(&tmp_err)) == 0){
                parse_creation_deletion_result(req.getRequestCode(),  url, davix_scope_mv_str(), req.getAnswerContent());
        }

    }

    DAVIX_DEBUG(" move <-");
    checkDavixError(&tmp_err);
    return ret;
}



int internal_checksum(Context & c, const Uri & url, const RequestParams *p, std::string & checksm, const std::string & chk_algo){
    DAVIX_DEBUG(" -> checksum");
    int ret=-1;
    DavixError* tmp_err=NULL;
    RequestParams params(p);

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
                    return 0;
                }
            }

            // fallback on extension for checksum
            std::string digest;
            req.getAnswerHeader("Digest", digest);
            if (digest.empty() == false){
                size_t valueOffset = digest.find('=');
                if (valueOffset == std::string::npos
                        || compare_ncase(digest,0, valueOffset, chk_algo.c_str()) !=0)
                    throw DavixException(davix_scope_meta(), StatusCode::InvalidServerResponse, "Invalid server checksum answer");

                digest.erase(digest.begin(), digest.begin()+valueOffset+1);
                std::swap(checksm, digest);
                return 0;
            }

            // last chance try to extract MD5 checksum from ETAG ( S3 work around )
            std::string etag_str;
            if(compare_ncase(chk_algo, "MD5") ==0 && req.getAnswerHeader("etag", etag_str)){
                stringVec tokens = tokenSplit(etag_str, "&;\\/\"'");
                for(stringVec::iterator it = tokens.begin(); it < tokens.end(); it++){
                    if(it->size() == 32 && std::find_if(it->begin(), it->end(), std::not1(StrUtil::isHexa())) == it->end()){
                        std::swap(checksm, *it);
                        return 0;
                    }
                }
            }

           std::ostringstream ss;
           ss << "checksum calculation for " << chk_algo << "not supported for " << url;
           throw DavixException(davix_scope_meta(), StatusCode::OperationNonSupported, ss.str());

           DAVIX_DEBUG(" checksum <-");
           return 0;
        }
    }
    throw DavixException(&tmp_err);
}


dav_ssize_t incremental_listdir_parsing(HttpRequest* req, XMLPropParser * parser, dav_size_t s_buff, const std::string & scope){
  //  std::cout << "time 1 pre-fecth" << time(NULL) << std::endl;
    DavixError* tmp_err=NULL;

    char buffer[s_buff+1];
    const dav_ssize_t ret = req->readSegment(buffer, s_buff, &tmp_err);
    checkDavixError(&tmp_err);
    if(ret >= 0){
        buffer[ret]= '\0';
        DAVIX_DEBUG("Listdir::ChunkParsing content : %s", buffer);
        parser->parseChunk(buffer, ret);
    }else{
        throw DavixException(scope, StatusCode::UnknowError, "Unknow readSegment error");
    }

    return ret;
}

bool wedav_get_next_property(Ptr::Scoped<DirHandle> & handle, std::string & name_entry, StatInfo & info){
    DAVIX_DEBUG(" -> wedav_get_next_property");
    const size_t read_size = 2048;


    HttpRequest& req = *(handle->request); // setup env again
    XMLPropParser& parser = *(handle->parser);

    size_t prop_size = parser.getProperties().size();
    ssize_t s_resu = read_size;

    while( prop_size == 0
          && s_resu > 0){ // request not complete and current data too smalls
        // continue the parsing until one more result
       s_resu = incremental_listdir_parsing(&req, &parser, read_size, "WebDav::listing");

       prop_size = parser.getProperties().size();
    }


    if(prop_size == 0){
        return false; // end of the request, end of the story
    }

    FileProperties & front = parser.getProperties().front();
    name_entry.swap(front.filename);
    info = front.info;
    parser.getProperties().pop_front(); // clean the current element
    return true;
}


void webdav_start_listing_query(Ptr::Scoped<DirHandle> & handle, Context & context, const RequestParams* params, const Uri & url, const std::string & body){
    dav_ssize_t s_resu;

    DavixError* tmp_err=NULL;
    handle.reset(new DirHandle(new PropfindRequest(context, url, &tmp_err), new DavPropXMLParser()));
    checkDavixError(&tmp_err);


    const int operation_timeout = params->getOperationTimeout()->tv_sec;
    HttpRequest & http_req = *(handle->request);
    XMLPropParser & parser = *(handle->parser);

    http_req.addHeaderField("Depth","1");
    time_t timestamp_timeout = time(NULL) + ((operation_timeout)?(operation_timeout):180);

    http_req.setParameters(params);
    // setup the handle for simple listing only
    http_req.setRequestBody(body);

    http_req.beginRequest(&tmp_err);
    checkDavixError(&tmp_err);

    check_file_status(http_req, davix_scope_directory_listing_str());

    size_t prop_size = 0;
    do{ // parse the begining of the request until the first property -> directory property
       s_resu = incremental_listdir_parsing(&http_req, &parser, 2048, davix_scope_directory_listing_str());

       prop_size = parser.getProperties().size();
       if(s_resu < 2048 && prop_size <1){ // verify request status : if req done + no data -> error
           throw DavixException(davix_scope_directory_listing_str(), StatusCode::WebDavPropertiesParsingError, "bad server answer, not a valid WebDav PROPFIND answer");
       }
       if(timestamp_timeout < time(NULL)){
          throw DavixException(davix_scope_directory_listing_str(), StatusCode::OperationTimeout, "operation timeout triggered while directory listing");
       }

    }while( prop_size < 1); // leave is end of req & no data

    const StatInfo & info = parser.getProperties().at(0).info;
    if( S_ISDIR(info.mode) == false){
        std::ostringstream ss;
        ss << url << " is not a collection, listing impossible";
        throw DavixException(davix_scope_directory_listing_str(), StatusCode::IsNotADirectory, ss.str());
    }else{
        parser.getProperties().pop_front(); // suppress the parent directory infos...
    }

}

bool webdav_directory_listing(Ptr::Scoped<DirHandle> & handle, Context & context, const RequestParams* params, const Uri & uri, const std::string & body, std::string & name_entry, StatInfo & info){
    if(handle.get() == NULL){
        webdav_start_listing_query(handle, context, params, uri, body);
    }
    return wedav_get_next_property(handle, name_entry, info);
}

HttpMetaOps::HttpMetaOps(): HttpIOChain(){}

HttpMetaOps::~HttpMetaOps(){}


void HttpMetaOps::checksum(IOChainContext & iocontext, std::string &checksm, const std::string &chk_algo){
    internal_checksum(iocontext._context, iocontext._uri, iocontext._reqparams, checksm, chk_algo);
}

void HttpMetaOps::makeCollection(IOChainContext & iocontext){
    internal_make_collection(iocontext._context, iocontext._uri, iocontext._reqparams);
}

void HttpMetaOps::move(IOChainContext & iocontext, const std::string & target_url){
    internal_move(iocontext._context, iocontext._uri, iocontext._reqparams, target_url);
}

void HttpMetaOps::deleteResource(IOChainContext & iocontext){
    internal_delete_resource(iocontext._context, iocontext._uri, iocontext._reqparams);
}

StatInfo & HttpMetaOps::statInfo(IOChainContext & iocontext, StatInfo &st_info){
    struct stat st;
    memset(&st, 0, sizeof(struct stat));
    getStatInfo(iocontext._context, iocontext._uri, iocontext._reqparams, st_info);
    return st_info;
}

bool HttpMetaOps::nextSubItem(IOChainContext &iocontext, std::string &entry_name, StatInfo &info){
    return webdav_directory_listing(directoryItem, iocontext._context, iocontext._reqparams, iocontext._uri, stat_listing,
                             entry_name, info);
}

/////////////////////////
/////////////////////////


bool is_a_bucket(const Uri & u){
    const std::string & s = u.getPath();
    return (std::find_if(s.begin(), s.end(), std::not1(StrUtil::isSlash())) == s.end()); // false if pathname does not match '\/+'
}



S3MetaOps::S3MetaOps() : HttpIOChain()
 {}

S3MetaOps::~S3MetaOps(){}

static bool is_s3_operation(IOChainContext & context){
    const std::string & proto = context._uri.getProtocol();
    const RequestProtocol::Protocol protocol_flag = context._reqparams->getProtocol();
    return ( proto.compare(0, 2, "s3") ==0 || protocol_flag == RequestProtocol::AwsS3);
}

void internal_s3_creat_bucket(Context & c, const Uri & url, const RequestParams & params){
    DavixError * tmp_err=NULL;
    PutRequest req(c, url, &tmp_err);
    checkDavixError(&tmp_err);

    req.setParameters(params);
    if( req.executeRequest(&tmp_err) < 0){
        const int code = req.getRequestCode();
        httpcodeToDavixException(code, davix_scope_meta(), "bucket creation failure");
    }

    checkDavixError(&tmp_err);
}


void S3MetaOps::checksum(IOChainContext &iocontext, std::string &checksm, const std::string &chk_algo){
    internal_checksum(iocontext._context, iocontext._uri, iocontext._reqparams, checksm, chk_algo);
}

void S3MetaOps::makeCollection(IOChainContext &iocontext){

    if(is_s3_operation(iocontext)){
        internal_s3_creat_bucket( iocontext._context, iocontext._uri, iocontext._reqparams);
    }else{
        HttpIOChain::makeCollection(iocontext);
    }
}

// get statInfo
StatInfo & S3MetaOps::statInfo(IOChainContext & iocontext, StatInfo & st_info){
    StatInfo & ref = HttpIOChain::statInfo(iocontext, st_info);

    if(is_s3_operation(iocontext) && is_a_bucket(iocontext._uri)){
        ref.mode |= S_IFDIR;
        ref.mode &= ~(S_IFREG);
    }
    return ref;
}




bool s3_get_next_property(Ptr::Scoped<DirHandle> & handle, std::string & name_entry, StatInfo & info){
    DAVIX_DEBUG(" -> s3_get_next_property");
    const size_t read_size = 2048;


    HttpRequest& req = *(handle->request); // setup env again
    XMLPropParser& parser = *(handle->parser);

    size_t prop_size = parser.getProperties().size();
    ssize_t s_resu = read_size;

    while( prop_size == 0
          && s_resu > 0){ // execute request only if no property are available

        // continue the parsing until one more result
       s_resu = incremental_listdir_parsing(&req, &parser, read_size, "S3::listing");
       prop_size = parser.getProperties().size();
    }


    if(prop_size == 0){
        return false; // end of the request, end of the story
    }

    FileProperties & front = parser.getProperties().front();
    name_entry.swap(front.filename);
    info = front.info;
    parser.getProperties().pop_front(); // clean the current element
    return true;
}


void s3_start_listing_query(Ptr::Scoped<DirHandle> & handle, Context & context, const RequestParams* params, const Uri & url, const std::string & body){
    (void) body;
    dav_ssize_t s_resu;

    DavixError* tmp_err=NULL;
    handle.reset(new DirHandle(new GetRequest(context, url, &tmp_err), new S3PropParser()));
    checkDavixError(&tmp_err);


    const int operation_timeout = params->getOperationTimeout()->tv_sec;
    HttpRequest & http_req = *(handle->request);
    XMLPropParser & parser = *(handle->parser);

    time_t timestamp_timeout = time(NULL) + ((operation_timeout)?(operation_timeout):180);

    http_req.setParameters(params);

    http_req.beginRequest(&tmp_err);
    checkDavixError(&tmp_err);

    check_file_status(http_req, davix_scope_directory_listing_str());

    size_t prop_size = 0;
    do{ // first entry -> bucket informations
       s_resu = incremental_listdir_parsing(&http_req, &parser, 2048, davix_scope_directory_listing_str());

       prop_size = parser.getProperties().size();
       if(s_resu < 2048 && prop_size <1){ // verify request status : if req done + no data -> error
           throw DavixException(davix_scope_directory_listing_str(), StatusCode::ParsingError, "Invalid server response, not a S3 listing");
       }
       if(timestamp_timeout < time(NULL)){
          throw DavixException(davix_scope_directory_listing_str(), StatusCode::OperationTimeout, "Operation timeout triggered while directory listing");
       }

    }while( prop_size < 1); // prop < 1 means not enough data

    const StatInfo & info = parser.getProperties().at(0).info;
    if( S_ISDIR(info.mode) == false){
        std::ostringstream ss;
        ss << url << " is not a S3 bucket";
        throw DavixException(davix_scope_directory_listing_str(), StatusCode::IsNotADirectory, ss.str());
    }else{
        parser.getProperties().pop_front(); // suppress the bucket entry
    }

}



bool s3_directory_listing(Ptr::Scoped<DirHandle> & handle, Context & context, const RequestParams* params, const Uri & uri, const std::string & body, std::string & name_entry, StatInfo & info){
    if(handle.get() == NULL){
        s3_start_listing_query(handle, context, params, uri, body);
    }
    return s3_get_next_property(handle, name_entry, info);
}


bool S3MetaOps::nextSubItem(IOChainContext &iocontext, std::string &entry_name, StatInfo &info){
    if(is_s3_operation(iocontext)){
        if(is_a_bucket(iocontext._uri) == false){
            throw DavixException(davix_scope_directory_listing_str(), StatusCode::IsNotADirectory, "This is not a S3 bucket");
        }
        return s3_directory_listing(directoryItem, iocontext._context, iocontext._reqparams, iocontext._uri, stat_listing,
                                 entry_name, info);
    }else{
        return HttpIOChain::nextSubItem(iocontext, entry_name, info);
    }

}





} // Davix
