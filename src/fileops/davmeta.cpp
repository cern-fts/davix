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

#include <davix_internal.hpp>
#include "davmeta.hpp"

#include <xml/davpropxmlparser.hpp>
#include <xml/davdeletexmlparser.hpp>
#include <xml/metalinkparser.hpp>
#include <xml/s3propparser.hpp>
#include <xml/azurepropparser.hpp>
#include <xml/swiftpropparser.hpp>

#include <utils/davix_logger_internal.hpp>
#include <utils/davix_utils_internal.hpp>
#include <utils/davix_s3_utils.hpp>
#include <utils/davix_azure_utils.hpp>
#include <utils/davix_gcloud_utils.hpp>
#include <utils/davix_swift_utils.hpp>
#include <utils/checksum_extractor.hpp>

#include <request/httprequest.hpp>
#include <fileops/fileutils.hpp>
#include <utils/stringutils.hpp>
#include "libs/alibxx/crypto/base64.hpp"
#include <neon/neonrequest.hpp>


using namespace StrUtil;

namespace Davix{

static const std::string stat_listing("<?xml version=\"1.0\" encoding=\"utf-8\" ?><D:propfind xmlns:D=\"DAV:\" xmlns:L=\"LCGDM:\"><D:prop>"
                                      "<D:displayname/><D:getlastmodified/><D:creationdate/><D:getcontentlength/><D:quota-used-bytes/>"
                                      "<D:resourcetype><D:collection/></D:resourcetype><L:mode/>"
                                      "<D:owner></D:owner><D:group></D:group>"
                                      "</D:prop>"
                                      "</D:propfind>");

static const std::string quota_stat("<?xml version=\"1.0\" encoding=\"utf-8\" ?><D:propfind xmlns:D=\"DAV:\" xmlns:L=\"LCGDM:\"><D:prop>"
                                      "<D:quota-used-bytes/><D:quota-available-bytes/>"
                                      "</D:prop>"
                                      "</D:propfind>");

struct DirHandle{

    DirHandle(HttpRequest* req, XMLPropParser * p): request(req), parser(p){}

    std::unique_ptr<HttpRequest> request;
    std::unique_ptr<Davix::XMLPropParser> parser;


};

/**
  execute a propfind/stat request on a given HTTP request handle
  return a vector with the content of the request if success
*/
std::vector<char> req_webdav_propfind(HttpRequest* req, DavixError** err){
    int ret =-1;
    std::vector<char> res;

    req->addHeaderField("Depth","0");
    req->setRequestMethod("PROPFIND");

    if( (ret = req->executeRequest(err)) ==0){
        res.swap(req->getAnswerContentVec());
    }

    return res;
}


int dav_stat_mapper_webdav(Context &context, const RequestParams* params, const Uri & url, struct StatInfo& st_info){
    int ret =-1;

    DavPropXMLParser parser;
    DavixError * tmp_err=NULL;
    HttpRequest req(context, url, &tmp_err);

    if( tmp_err == NULL){
        req.setParameters(params);

        TRY_DAVIX{
            std::vector<char> body = req_webdav_propfind(&req, &tmp_err);
            if(!tmp_err){
               parser.parseChunk(&(body[0]), body.size());

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


// Implement stat with a GET of Range 1
int dav_stat_mapper_http_get(Context& context, const RequestParams* params, const Uri & uri, struct StatInfo& st_info){
    int ret = -1;
    DavixError * tmp_err=NULL;
    GetRequest req(context, uri, &tmp_err);

    if( tmp_err == NULL){
        req.setParameters(params);
        req.addHeaderField("Range", "bytes=0-1");
        req.executeRequest(&tmp_err);

        if(!tmp_err){
            if(httpcodeIsValid(req.getRequestCode()) ){
                memset(&st_info, 0, sizeof(struct StatInfo));
                std::string rnge;
                req.getAnswerHeader("Content-Range", rnge);
                std::string::size_type pos = rnge.find_first_of("/");
                if (pos == std::string::npos) {
                    throw DavixException(davix_scope_meta(), StatusCode::ParsingError, "Content-Range not parsable");
                }
                if (rnge.substr(pos+1,1) == "*") {
                   throw DavixException(davix_scope_meta(), StatusCode::ParsingError, "Server does not provide content length");
                }
                long lsize = toType<long, std::string>()(rnge.substr(pos+1));
                st_info.size = std::max<long>(0,lsize);
                st_info.mode = 0755 | S_IFREG;
                req.discardBody(&tmp_err);
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


dav_ssize_t incremental_listdir_parsing(HttpRequest* req, XMLPropParser * parser, dav_size_t s_buff, const std::string & scope){
    DavixError* tmp_err=NULL;

    char buffer[s_buff+1];
    const dav_ssize_t ret = req->readSegment(buffer, s_buff, &tmp_err);
    checkDavixError(&tmp_err);
    if(ret >= 0){
        buffer[ret]= '\0';
        parser->parseChunk(buffer, ret);
    }else{
        throw DavixException(scope, StatusCode::UnknownError, "Unknown readSegment error");
    }

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
            if (isS3SignedURL(url)) {
                // This endpoint won't accept a HEAD request, use GET instead
                ret = dav_stat_mapper_http_get(c, &params, url, st_info);
            } else {
                ret = dav_stat_mapper_http(c, &params, url, st_info);
            }
            break;

    }
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, " davix_stat <-");
    return ret;
}

QuotaInfo::QuotaInfo() : d_ptr(new Internal()) { }
QuotaInfo::~QuotaInfo() { }
dav_size_t QuotaInfo::getUsedBytes() {
    return d_ptr->used_bytes;
}
dav_size_t QuotaInfo::getFreeSpace() {
    return d_ptr->free_space;
}

class QuotaInfoHandler {
public:
    static void setdptr(QuotaInfo &info, QuotaInfo::Internal &internal) {
        info.d_ptr.reset(new QuotaInfo::Internal(internal));
    }
};

void getQuotaInfo(Context & c, const Uri & url, const RequestParams *p, QuotaInfo &info) {
    DavixError * tmp_err = NULL;
    HttpRequest req(c, url, &tmp_err);
    checkDavixError(&tmp_err);

    req.setParameters(*p);
    req.addHeaderField("Depth","0");
    req.setRequestMethod("PROPFIND");
    req.setRequestBody(quota_stat);

    if(req.executeRequest(&tmp_err) == 0 && !tmp_err) {
        DavPropXMLParser parser;
        parser.parseChunk(&(req.getAnswerContentVec()[0]), req.getAnswerContentVec().size());
        std::deque<FileProperties> & props = parser.getProperties();
        if( props.size() < 1){
            throw DavixException(davix_scope_stat_str(), Davix::StatusCode::WebDavPropertiesParsingError, "Parsing Error : properties number < 1");
        }else{
            QuotaInfoHandler::setdptr(info, props.front().quota);
        }
    }

    checkDavixError(&tmp_err);
}

void parse_creation_deletion_result(int code, const Uri & u, const std::string & scope, const std::vector<char> & body){
    switch(code){
        case 200:
        case 201:
        case 202:
        case 204:{
                return;
        }
        case 207:{
            // parse webdav
            DavDeleteXMLParser parser;
            parser.parseChunk(&(body[0]), body.size());
            if( parser.getProperties().size() > 0){
                for(unsigned int i=0; i < parser.getProperties().size(); ++i){
                   const int sub_code = parser.getProperties().at(i).req_status;
                   std::ostringstream ss;

                   ss << "occurred during deletion request for " << parser.getProperties().at(i).filename;

                   if(httpcodeIsValid(sub_code) == false){
                       httpcodeToDavixException(sub_code, scope, ss.str());
                   }
                }

               return;
            }
            // if no properties, properties were filtered because invalid
            httpcodeToDavixException(404, scope);
            break;
        }
    }
    std::ostringstream ss;
    ss << " with url " << u.getString();
    httpcodeToDavixException(code, scope, ss.str());
}


int internal_delete_resource(Context & c, const Uri & url, const RequestParams & params){
    DavixError* tmp_err=NULL;
    int ret=-1;
    RequestParams _params(params);

    DeleteRequest req(c, url, &tmp_err);
    req.setParameters(_params);
    if(!tmp_err){
         if( ( ret=req.executeRequest(&tmp_err)) == 0){
                parse_creation_deletion_result(req.getRequestCode(), url, davix_scope_rm_str(), req.getAnswerContentVec());
         }
    }


    checkDavixError(&tmp_err);
    return ret;
}


int internal_make_collection(Context & c, const Uri & url, const RequestParams & params){
    DAVIX_SCOPE_TRACE(DAVIX_LOG_CHAIN, mkcoll);

    int ret=-1;
    DavixError* tmp_err=NULL;
    RequestParams _params(params);

    HttpRequest req(c, url, &tmp_err);

    if(tmp_err == NULL){
        req.setParameters(params);
        req.setRequestMethod("MKCOL");
        if( (ret = req.executeRequest(&tmp_err)) == 0){
                parse_creation_deletion_result(req.getRequestCode(),  url, davix_scope_mkdir_str(), req.getAnswerContentVec());
        }

    }

    checkDavixError(&tmp_err);
    return ret;
}


int internal_move(Context & c, const Uri & url, const RequestParams & params, const std::string & target_url){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, " -> move");
    int ret=-1;
    DavixError* tmp_err=NULL;
    RequestParams _params(params);

    HttpRequest req(c, url, &tmp_err);

    if(tmp_err == NULL){
        req.setParameters(params);
        req.setRequestMethod("MOVE");

        Uri uri(target_url);
        uri.httpizeProtocol();
        req.addHeaderField("Destination", uri.getString());

        if( (ret = req.executeRequest(&tmp_err)) == 0){
                parse_creation_deletion_result(req.getRequestCode(),  url, davix_scope_mv_str(), req.getAnswerContentVec());
        }

    }

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, " move <-");
    checkDavixError(&tmp_err);
    return ret;
}



int internal_checksum(Context & c, const Uri & url, const RequestParams *p, std::string & checksm, const std::string & chk_algo){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, " -> checksum");
    int ret=-1;
    DavixError* tmp_err=NULL;
    RequestParams params(p);

    HeadRequest req(c, url, &tmp_err);

    if(tmp_err == NULL){
        // add Digest file, support for other digest, extended format
        req.addHeaderField("Want-Digest", chk_algo);
        req.setParameters(params);
        if( (ret = req.executeRequest(&tmp_err)) == 0
            && !tmp_err && (ret = davixRequestToFileStatus(&req, davix_scope_mkdir_str(), &tmp_err)) >=0){

            // try simple MD5 ( standard )
            if(compare_ncase(chk_algo, "MD5") == 0){
                std::string  chk;
                if(req.getAnswerHeader("Content-MD5", chk) == true){
                    DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CHAIN, "Extract MD5 checksum in base64 {}", chk);
                    chk= Base64::base64_decode(chk);
                    std::swap(checksm, chk);
                    return 0;
                }
            }

            // fallback on extension for checksum
            HeaderVec headers;
            req.getAnswerHeaders(headers);

            if(ChecksumExtractor::extractChecksum(headers, chk_algo, checksm)) {
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
           ss << "checksum calculation for " << chk_algo << " not supported for " << url;
           throw DavixException(davix_scope_meta(), StatusCode::OperationNonSupported, ss.str());

           DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, " checksum <-");
           return 0;
        }
    }
    throw DavixException(&tmp_err);
}




bool wedav_get_next_property(std::unique_ptr<DirHandle> & handle, std::string & name_entry, StatInfo & info){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, " -> wedav_get_next_property");
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


void webdav_start_listing_query(std::unique_ptr<DirHandle> & handle, Context & context, const RequestParams* params, const Uri & url, const std::string & body){
    dav_ssize_t s_resu;

    DavixError* tmp_err=NULL;
    handle.reset(new DirHandle(new PropfindRequest(context, url, &tmp_err), new DavPropXMLParser()));
    checkDavixError(&tmp_err);

    HttpRequest & http_req = *(handle->request);
    XMLPropParser & parser = *(handle->parser);

    http_req.addHeaderField("Depth","1");
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

bool webdav_directory_listing(std::unique_ptr<DirHandle> & handle, Context & context, const RequestParams* params, const Uri & uri, const std::string & body, std::string & name_entry, StatInfo & info){
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

QuotaInfo & HttpMetaOps::quotaInfo(IOChainContext & iocontext, QuotaInfo &info) {
    getQuotaInfo(iocontext._context, iocontext._uri, iocontext._reqparams, info);
    return info;
}

bool HttpMetaOps::nextSubItem(IOChainContext &iocontext, std::string &entry_name, StatInfo &info){
    return webdav_directory_listing(directoryItem, iocontext._context, iocontext._reqparams, iocontext._uri, stat_listing,
                             entry_name, info);
}

/////////////////////////
/////////////////////////

SwiftMetaOps::SwiftMetaOps() : HttpIOChain()
{}

SwiftMetaOps::~SwiftMetaOps(){}

static bool is_swift_operation(IOChainContext & context){
    return context._reqparams->getProtocol() == RequestProtocol::Swift;
}

static void swiftStatMapper(Context& context, const RequestParams* params, const Uri & uri, struct StatInfo& st_info) {
    const std::string scope = "Davix::swiftStatMapper";
    DavixError * tmp_err=NULL;
    HeadRequest req(context, uri, &tmp_err);

    // we need to modify it, hence copy
    RequestParams p(params);

    if(tmp_err == NULL) {
        req.setParameters(p);
        req.executeRequest(&tmp_err);
        const int code = req.getRequestCode();

        if(code == 404) {
            DavixError::clearError(&tmp_err);
            // try to "list" target resource and see if there is anything inside it, if there is, then it's a directory
            Uri new_url = Swift::swiftUriTransformer(uri, p, true);

            GetRequest http_req(context, new_url, &tmp_err);

            http_req.setParameters(p);

            http_req.beginRequest(&tmp_err);
            checkDavixError(&tmp_err);

            check_file_status(http_req, scope);

            // check response text, if there is data, then it is a directory, otherwise not a directory or file
            char buffer[256+1];
            const dav_ssize_t ret = http_req.readSegment(buffer, 256, &tmp_err);
            checkDavixError(&tmp_err);
            if(ret == 0){
                throw DavixException(scope, StatusCode::FileNotFound, "Not a file or directory");
            }
            else if (ret < 0) {
                throw DavixException(scope, StatusCode::UnknownError, "Unknown readSegment error");
            }
            checkDavixError(&tmp_err);

            st_info.mode = 0755;
            st_info.mode |= S_IFDIR;
        }
        else if(code == 200){
            st_info.mode = 0755;

            std::string swift_path = Swift::extract_swift_path(uri);
            if(swift_path == "/") // is container
                st_info.mode |= S_IFDIR;
            else if(swift_path[swift_path.size()-1] == '/' && req.getAnswerSize() == 0) { // is a directory
                st_info.mode |= S_IFDIR;
            }
            else {   // is file
                st_info.mode |= S_IFREG;
                const dav_ssize_t s = req.getAnswerSize();
                st_info.size = std::max<dav_ssize_t>(0, s);
                st_info.mtime = req.getLastModified();
            }
        }
        else if(code == 204){ // Normal response of a HEAD request to a container is 204
            st_info.mode = 0755;

            std::string swift_path = Swift::extract_swift_path(uri);
            if(swift_path == "/") // is container
                st_info.mode |= S_IFDIR;
        }
        else if(code == 500){
            throw DavixException(scope, StatusCode::UnknownError, "Internal Server Error triggered while attempting to get Swift object's stats");
        }
    }
    checkDavixError(&tmp_err);
}

StatInfo & SwiftMetaOps::statInfo(IOChainContext &iocontext, StatInfo &st_info) {
    if(is_swift_operation(iocontext)){
        swiftStatMapper(iocontext._context, iocontext._reqparams, iocontext._uri, st_info);
        return st_info;
    }
    else{
        StatInfo & ref = HttpIOChain::statInfo(iocontext, st_info);
        return ref;
    }
}

bool is_a_container(Uri u) {
    std::string tmp = Swift::extract_swift_path(u);
    if(tmp.compare("/") == 0){
        return true;
    }
    return false;
}

bool s3_get_next_property(std::unique_ptr<DirHandle> & handle, std::string & name_entry, StatInfo & info);

void swift_start_listing_query(std::unique_ptr<DirHandle> & handle, Context & context, const RequestParams* params, const Uri & url, const std::string & body){
    (void) body;
    dav_ssize_t s_resu;
    DavixError* tmp_err=NULL;

    if(params->getSwiftListingMode() == SwiftListingMode::Hierarchical){
        Uri new_url = Swift::swiftUriTransformer(url, params, true);
        handle.reset(new DirHandle(new GetRequest(context, new_url, &tmp_err), new SwiftPropParser(Swift::extract_swift_path(url))));
    }
    else if(params->getSwiftListingMode() == SwiftListingMode::SemiHierarchical){
        Uri new_url = Swift::swiftUriTransformer(url, params, false);
        handle.reset(new DirHandle(new GetRequest(context, new_url, &tmp_err), new SwiftPropParser(Swift::extract_swift_path(url))));
    }
    else{
        if(is_a_container(url) == false){
            throw DavixException(davix_scope_directory_listing_str(), StatusCode::IsNotADirectory, "This is not a Swift container");
        }
        handle.reset(new DirHandle(new GetRequest(context, url, &tmp_err), new SwiftPropParser()));
    }
    checkDavixError(&tmp_err);


    const int operation_timeout = params->getOperationTimeout()->tv_sec;
    HttpRequest & http_req = *(handle->request);
    XMLPropParser & parser = *(handle->parser);

    time_t timestamp_timeout = time(NULL) + ((operation_timeout)?(operation_timeout):180);

    http_req.setParameters(params);
    http_req.addHeaderField("Accept", "application/xml");

    http_req.beginRequest(&tmp_err);
    checkDavixError(&tmp_err);

    check_file_status(http_req, davix_scope_directory_listing_str());

    size_t prop_size = 0;
    do{ // first entry -> container information
        s_resu = incremental_listdir_parsing(&http_req, &parser, 2048, davix_scope_directory_listing_str());

        prop_size = parser.getProperties().size();
        if(s_resu < 2048 && prop_size <1){ // verify request status : if req done + no data -> error
            throw DavixException(davix_scope_directory_listing_str(), StatusCode::ParsingError, "Invalid server response, not a Swift listing or the directory is empty");
        }
        if(timestamp_timeout < time(NULL)){
            throw DavixException(davix_scope_directory_listing_str(), StatusCode::OperationTimeout, "Operation timeout triggered while directory listing");
        }

    }while( prop_size < 1); // prop < 1 means not enough data

}

bool swift_directory_listing(std::unique_ptr<DirHandle> & handle, Context & context, const RequestParams* params, const Uri & uri, const std::string & body, std::string & name_entry, StatInfo & info){
    if(handle.get() == NULL){
        swift_start_listing_query(handle, context, params, uri, body);
    }
    return s3_get_next_property(handle, name_entry, info);
}


bool SwiftMetaOps::nextSubItem(IOChainContext &iocontext, std::string &entry_name, StatInfo &info){
    if(is_swift_operation(iocontext)){
        return swift_directory_listing(directoryItem, iocontext._context, iocontext._reqparams, iocontext._uri, stat_listing,
                                    entry_name, info);
    }else{
        return HttpIOChain::nextSubItem(iocontext, entry_name, info);
    }

}

void SwiftMetaOps::move(IOChainContext & iocontext, const std::string & target_url) {
    const std::string scope = "Davix::SwiftMetaOps::move";
    if(!is_swift_operation(iocontext)) {
        return HttpIOChain::move(iocontext, target_url);
    }

    Context context = iocontext._context;
    RequestParams params = iocontext._reqparams;
    Uri uri(iocontext._uri);
    Uri target(target_url);

    // verify both are using the same swift provider/server, reuse s3 utils
    std::string p1 = S3::extract_s3_provider(uri);
    std::string p2 = S3::extract_s3_provider(target);

    if(p1 != p2) {
        throw DavixException(scope, StatusCode::OperationNonSupported,
                             "It looks that the two URLs are not using the same Swift provider. Unable to perform the move operation.");
    }

    std::string source_container = Swift::extract_swift_container(uri);
    std::string source_path = Swift::extract_swift_path(uri);

    DavixError *tmp_err = NULL;
    PutRequest req(context, target, &tmp_err);
    checkDavixError(&tmp_err);
    req.setParameters(iocontext._reqparams);
    req.addHeaderField("X-Copy-From", "/" + source_container + source_path);

    req.executeRequest(&tmp_err);
    checkDavixError(&tmp_err);

    // if copying was successful, delete the source file
    if(req.getRequestCode() == 201) {

        DeleteRequest req(context, uri, &tmp_err);
        checkDavixError(&tmp_err);

        RequestParams p(iocontext._reqparams);
        req.setParameters(p);

        req.executeRequest(&tmp_err);
        checkDavixError(&tmp_err);
    }
    else {
        std::stringstream str;
        str << "Received code " << req.getRequestCode() << " when trying to copy file - will not perform deletion";
        throw DavixException(scope, StatusCode::UnknownError, str.str());
    }
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

    if( proto.compare(0, 2, "s3") ==0 || protocol_flag == RequestProtocol::AwsS3) return true;
    if( proto.compare(0, 6, "gcloud") ==0 || protocol_flag == RequestProtocol::Gcloud) return true;

    return false;
}

static void internal_s3_create_bucket_or_dir(Context & c, const Uri & url, const RequestParams & params){
    DavixError * tmp_err=NULL;

    /* make sure path ends with a slash, otherwise s3
       will just create a zero-length file */
    Uri url2 = url;
    if(url.getPath()[url.getPath().size()-1] != '/') {
        url2.setPath(url.getPath() + "/");
    }

    PutRequest req(c, url2, &tmp_err);
    req.addHeaderField("Content-Length", "0");
    checkDavixError(&tmp_err);

    req.setParameters(params);
    if( req.executeRequest(&tmp_err) < 0){
        const int code = req.getRequestCode();
        httpcodeToDavixException(code, davix_scope_meta(), "bucket creation failure");
    }

    checkDavixError(&tmp_err);
}

void S3MetaOps::move(IOChainContext & iocontext, const std::string & target_url) {
    const std::string scope = "Davix::S3MetaOps::move";
    if(!is_s3_operation(iocontext)) {
        return HttpIOChain::move(iocontext, target_url);
    }

    Context context = iocontext._context;
    RequestParams params = iocontext._reqparams;
    Uri uri(iocontext._uri);
    Uri target(target_url);

    // verify both are using the same s3 provider/server
    std::string p1 = S3::extract_s3_provider(uri);
    std::string p2 = S3::extract_s3_provider(target);

    if(p1 != p2) {
        throw DavixException(scope, StatusCode::OperationNonSupported,
                             "It looks that the two URLs are not using the same S3 provider. Unable to perform the move operation.");
    }

    std::string source_bucket = S3::extract_s3_bucket(uri, params.getAwsAlternate());
    std::string source_path = S3::extract_s3_path(uri, params.getAwsAlternate());

    DavixError *tmp_err = NULL;
    PutRequest req(context, target, &tmp_err);
    checkDavixError(&tmp_err);
    req.setParameters(iocontext._reqparams);
    req.addHeaderField("x-amz-copy-source", "/" + source_bucket + source_path);

    req.executeRequest(&tmp_err);
    checkDavixError(&tmp_err);

    // if copying was successful, delete the source file
    if(req.getRequestCode() == 200) {
        std::string region = S3::detect_region(uri);
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Detected region for source endpoint: " + region);
        checkDavixError(&tmp_err);

        DeleteRequest req(context, uri, &tmp_err);
        checkDavixError(&tmp_err);

        RequestParams p(iocontext._reqparams);
        p.setAwsRegion(region);
        req.setParameters(p);

        req.executeRequest(&tmp_err);
        checkDavixError(&tmp_err);
    }
    else {
        std::stringstream str;
        str << "Received code " << req.getRequestCode() << " when trying to copy file - will not perform deletion";
        throw DavixException(scope, StatusCode::UnknownError, str.str());
    }
}


void S3MetaOps::checksum(IOChainContext &iocontext, std::string &checksm, const std::string &chk_algo){
    internal_checksum(iocontext._context, iocontext._uri, iocontext._reqparams, checksm, chk_algo);
}

void S3MetaOps::makeCollection(IOChainContext &iocontext){
    if(is_s3_operation(iocontext) || is_swift_operation(iocontext)){
        internal_s3_create_bucket_or_dir( iocontext._context, iocontext._uri, iocontext._reqparams);
    }else{
        HttpIOChain::makeCollection(iocontext);
    }
}


void s3StatMapper(Context& context, const RequestParams* params, const Uri & uri, struct StatInfo& st_info){
    const std::string scope = "Davix::s3StatMapper";
    DavixError * tmp_err=NULL;
    HeadRequest req(context, uri, &tmp_err);

    // we need to modify it, hence copy
    RequestParams p(params);
    // we just need to know if target has anything inside it
    p.setS3MaxKey(1);

    if( tmp_err == NULL){
        req.setParameters(p);
        req.executeRequest(&tmp_err);
        const int code = req.getRequestCode();

        // if 404, target either doesn't exist or is a S3 "directory"
        if(code == 404){
            DavixError::clearError(&tmp_err);
            // try to "list" target resource and see if there is anything inside it, if there is, then it's a directory
            Uri new_url = S3::s3UriTransformer(uri, p, true);
            DirHandle handle(new GetRequest(context, new_url, &tmp_err), new S3PropParser(params->getS3ListingMode(), S3::extract_s3_path(uri, params->getAwsAlternate())));

            dav_ssize_t s_resu=0;

            const int operation_timeout = p.getOperationTimeout()->tv_sec;
            HttpRequest & http_req = *(handle.request);
            XMLPropParser & parser = *(handle.parser);

            time_t timestamp_timeout = time(NULL) + ((operation_timeout)?(operation_timeout):180);

            http_req.setParameters(p);

            http_req.beginRequest(&tmp_err);
            checkDavixError(&tmp_err);

            check_file_status(http_req, scope);

            size_t prop_size = 0;
            do{ // first entry
               TRY_DAVIX{
                    s_resu = incremental_listdir_parsing(&http_req, &parser, 2048, scope);
               }CATCH_DAVIX(&tmp_err)

               if(tmp_err && (tmp_err->getStatus() == StatusCode::IsNotADirectory)){
                  std::ostringstream ss;
                  ss << uri << " not found";
                  throw DavixException(scope, StatusCode::FileNotFound, ss.str());
                }

               prop_size = parser.getProperties().size();
               if(s_resu < 2048 && prop_size <1){ // verify request status : if req done + no data -> error
                  throw DavixException(scope, StatusCode::ParsingError, "Invalid server response, not a S3 listing");
               }
               if(timestamp_timeout < time(NULL)){
                  throw DavixException(scope, StatusCode::OperationTimeout, "Operation timeout triggered while getting S3 object's stats");
               }

            }while( prop_size < 1); // prop < 1 means not enough data

            st_info.mode = 0755;
            st_info.mode |= S_IFDIR;
        }
        else if(code == 200){ // found something, must be a file not directory
            st_info.mode = 0755;

            std::string s3_path = S3::extract_s3_path(uri, params->getAwsAlternate());
            if(s3_path == "/") // is bucket
                st_info.mode |= S_IFDIR;
            else if(s3_path[s3_path.size()-1] == '/' && req.getAnswerSize() == 0) { // is a directory
                st_info.mode |= S_IFDIR;
            }
            else{   // is file
                st_info.mode |= S_IFREG;
                const dav_ssize_t s = req.getAnswerSize();
                st_info.size = std::max<dav_ssize_t>(0,s);
                st_info.mtime = req.getLastModified();
            }
        }
        else if(code == 500)
            throw DavixException(scope, StatusCode::UnknownError, "Internal Server Error triggered while attempting to get S3 object's stats");
    }
    checkDavixError(&tmp_err);
}


// get statInfo
StatInfo & S3MetaOps::statInfo(IOChainContext & iocontext, StatInfo & st_info){
    if(is_s3_operation(iocontext)){
        s3StatMapper(iocontext._context, iocontext._reqparams, iocontext._uri, st_info);
        return st_info;
    }
    else{
        StatInfo & ref = HttpIOChain::statInfo(iocontext, st_info);
        return ref;
    }
}


bool s3_get_next_property(std::unique_ptr<DirHandle> & handle, std::string & name_entry, StatInfo & info){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, " -> s3_get_next_property");
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


void s3_start_listing_query(std::unique_ptr<DirHandle> & handle, Context & context, const RequestParams* params, const Uri & url, const std::string & body){
    (void) body;
    dav_ssize_t s_resu;
    DavixError* tmp_err=NULL;
    bool listing_buckets;

    if(params->getProtocol() == RequestProtocol::Gcloud) {
        Uri new_url = gcloud::getListingURI(url, params);

        std::string prefix = gcloud::extract_path(url);
        if(prefix != "/") prefix = "/" + prefix;
        handle.reset(new DirHandle(new GetRequest(context, new_url, &tmp_err), new S3PropParser(params->getS3ListingMode(),  prefix)));
    }
    else if(params->getS3ListingMode() == S3ListingMode::Hierarchical){
        Uri new_url = S3::s3UriTransformer(url, params, true);
        handle.reset(new DirHandle(new GetRequest(context, new_url, &tmp_err), new S3PropParser(params->getS3ListingMode(), S3::extract_s3_path(url, params->getAwsAlternate()))));
    }
    else if(params->getS3ListingMode() == S3ListingMode::SemiHierarchical){
        Uri new_url = S3::s3UriTransformer(url, params, false);
        handle.reset(new DirHandle(new GetRequest(context, new_url, &tmp_err), new S3PropParser(params->getS3ListingMode(), S3::extract_s3_path(url, params->getAwsAlternate()))));
    }
    else{
        if(is_a_bucket(url) == false){
           throw DavixException(davix_scope_directory_listing_str(), StatusCode::IsNotADirectory, "This is not a S3 bucket");
        }
        handle.reset(new DirHandle(new GetRequest(context, url, &tmp_err), new S3PropParser()));
    }
    checkDavixError(&tmp_err);

    // Check if we are listing available buckets
    listing_buckets = (params->getAwsAlternate() && url.getPath() == "/");


    const int operation_timeout = params->getOperationTimeout()->tv_sec;
    HttpRequest & http_req = *(handle->request);
    XMLPropParser & parser = *(handle->parser);

    time_t timestamp_timeout = time(NULL) + ((operation_timeout)?(operation_timeout):180);

    http_req.setParameters(params);

    http_req.beginRequest(&tmp_err);
    checkDavixError(&tmp_err);

    check_file_status(http_req, davix_scope_directory_listing_str());

    size_t prop_size = 0;
    do{ // first entry -> bucket information
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
    } else if (!listing_buckets) {
            parser.getProperties().pop_front(); // suppress the bucket name entry
    }

}



bool s3_directory_listing(std::unique_ptr<DirHandle> & handle, Context & context, const RequestParams* params, const Uri & uri, const std::string & body, std::string & name_entry, StatInfo & info){
    if(handle.get() == NULL){
        s3_start_listing_query(handle, context, params, uri, body);
    }
    return s3_get_next_property(handle, name_entry, info);
}


bool S3MetaOps::nextSubItem(IOChainContext &iocontext, std::string &entry_name, StatInfo &info){
    if(is_s3_operation(iocontext)){
        return s3_directory_listing(directoryItem, iocontext._context, iocontext._reqparams, iocontext._uri, stat_listing,
                                 entry_name, info);
    }else{
        return HttpIOChain::nextSubItem(iocontext, entry_name, info);
    }

}

/////////////////////////
/////////////////////////

AzureMetaOps::AzureMetaOps() : HttpIOChain()
 {}

AzureMetaOps::~AzureMetaOps(){}

static bool is_azure_operation(IOChainContext & context){
    return context._reqparams->getProtocol() == RequestProtocol::Azure;
}

void azureStatMapper(Context& context, const RequestParams* params, const Uri & uri, struct StatInfo& st_info) {
    const std::string scope = "Davix::azureStatMapper";
    DavixError * tmp_err=NULL;

    //Context c;
    HeadRequest req(context, uri, &tmp_err);

    // we need to modify it, hence copy
    RequestParams p(params);
    // we just need to know if target has anything inside it
    //p.setS3MaxKey(1);

    if( tmp_err == NULL){
        req.setParameters(p);
        req.executeRequest(&tmp_err);
        const int code = req.getRequestCode();

        // if 404, target either doesn't exist or is an Azure "directory". TODO: add support to stat directories. must have func tests by then
        if(code == 404){
            DavixError::clearError(&tmp_err);
            Uri new_url = Azure::transformURI(uri, p, true);
            DirHandle handle(new GetRequest(context, new_url, &tmp_err), new AzurePropParser(Azure::extract_azure_filename(uri)));

            dav_ssize_t s_resu=0;

            const int operation_timeout = p.getOperationTimeout()->tv_sec;
            HttpRequest & http_req = *(handle.request);
            XMLPropParser & parser = *(handle.parser);

            time_t timestamp_timeout = time(NULL) + ((operation_timeout)?(operation_timeout):180);

            http_req.setParameters(p);

            http_req.beginRequest(&tmp_err);
            checkDavixError(&tmp_err);
            check_file_status(http_req, scope);

            size_t prop_size = 0;
            do{ // first entry -> container information
                s_resu = incremental_listdir_parsing(&http_req, &parser, 2048, davix_scope_directory_listing_str());

                prop_size = parser.getProperties().size();
                if(s_resu < 2048 && prop_size <1){ // verify request status : if req done + no data -> error
                    throw DavixException(davix_scope_directory_listing_str(), StatusCode::IsNotADirectory, "The specified directory does not exist");
                }
                if(timestamp_timeout < time(NULL)){
                    throw DavixException(davix_scope_directory_listing_str(), StatusCode::OperationTimeout, "Operation timeout triggered while directory listing");
                }
            }while( prop_size < 1); // prop < 1 means not enough data

            st_info.mode = 0755;
            st_info.mode |= S_IFDIR;
        }
        // file exists, parse its info
        else if(code == 200) {
            st_info.mode = 0755;
            st_info.mode |= S_IFREG;
            const dav_ssize_t s = req.getAnswerSize();
            st_info.size = std::max<dav_ssize_t>(0,s);
            st_info.mtime = req.getLastModified();
        }

    }

}

StatInfo & AzureMetaOps::statInfo(IOChainContext & iocontext, StatInfo & st_info) {
    if(is_azure_operation(iocontext)) {
        azureStatMapper(iocontext._context, iocontext._reqparams, iocontext._uri, st_info);
        return st_info;
    }
    else {
        return HttpIOChain::statInfo(iocontext, st_info);
    }
}

static void azure_start_listing_query(std::unique_ptr<DirHandle> & handle, Context & context, const RequestParams* params, const Uri & url, const std::string & body) {
    DavixError* tmp_err=NULL;
    dav_ssize_t s_resu;

    Uri new_url = Davix::Azure::transformURI(url, params, true);
    handle.reset(new DirHandle(new GetRequest(context, new_url, &tmp_err), new AzurePropParser(Davix::Azure::extract_azure_filename(url))));

    const int operation_timeout = params->getOperationTimeout()->tv_sec;
    HttpRequest & http_req = *(handle->request);
    XMLPropParser & parser = *(handle->parser);
    time_t timestamp_timeout = time(NULL) + ((operation_timeout)?(operation_timeout):180);

    http_req.setParameters(params);

    http_req.beginRequest(&tmp_err);
    checkDavixError(&tmp_err);

    check_file_status(http_req, davix_scope_directory_listing_str());

    size_t prop_size = 0;
    do{ // first entry -> container information
       s_resu = incremental_listdir_parsing(&http_req, &parser, 2048, davix_scope_directory_listing_str());

       prop_size = parser.getProperties().size();
       if(s_resu < 2048 && prop_size <1){ // verify request status : if req done + no data -> error
           throw DavixException(davix_scope_directory_listing_str(), StatusCode::IsNotADirectory, "The specified directory does not exist");
       }
       if(timestamp_timeout < time(NULL)){
          throw DavixException(davix_scope_directory_listing_str(), StatusCode::OperationTimeout, "Operation timeout triggered while directory listing");
       }
    }while( prop_size < 1); // prop < 1 means not enough data

}

bool azure_get_next_property(std::unique_ptr<DirHandle> & handle, std::string & name_entry, StatInfo & info) {
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, " -> azure_get_next_property");
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

static bool azure_directory_listing(std::unique_ptr<DirHandle> & handle, Context & context, const RequestParams* params, const Uri & uri, const std::string & body, std::string & name_entry, StatInfo & info){
    if(handle.get() == NULL){
        azure_start_listing_query(handle, context, params, uri, body);
    }
    return azure_get_next_property(handle, name_entry, info);
}

bool AzureMetaOps::nextSubItem(IOChainContext &iocontext, std::string &entry_name, StatInfo &info) {
    if(is_azure_operation(iocontext)){
        return azure_directory_listing(directoryItem, iocontext._context, iocontext._reqparams, iocontext._uri, stat_listing,
                                 entry_name, info);
    }else{
        return HttpIOChain::nextSubItem(iocontext, entry_name, info);
    }
}


} // Davix
