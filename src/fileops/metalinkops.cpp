#include <davix_internal.hpp>
#include "metalinkops.hpp"

#include <string_utils/stringutils.hpp>
#include <utils/davix_logger_internal.hpp>
#include <xml/metalinkparser.hpp>
#include <base64/base64.hpp>


namespace Davix{

using namespace StrUtil;

int davix_metalink_header_parser(const std::string & header_key, const std::string & header_value,
                                 const Uri & u_original,
                                 Uri & metalink){
    DAVIX_TRACE("Parse headers for metalink %s %s", header_key.c_str(), header_value.c_str());

    if(compare_ncase(header_key, "Link") ==0 && header_value.find("application/metalink") != std::string::npos){
        std::string::const_iterator it1, it2;
        if( ( it1 = std::find(header_value.begin(), header_value.end(), '<')) != header_value.end()
                && ( it2 = std::find(it1, header_value.end(), '>')) != header_value.end()){
            std::string metalink_url(it1+1, it2);
            metalink =  Uri::fromRelativePath(u_original, trim(metalink_url));
            if(metalink.getStatus() == StatusCode::OK){
                DAVIX_TRACE("Valid metalink URI found %s", metalink.getString().c_str());
                return 1;
            }

        }
    }
    return 0;
}

bool davix_metalink_header_content_type(const std::string & header_key, const std::string & header_value){
    return (compare_ncase(header_key, "Content-type") ==0 &&  header_value.find("application/metalink") !=std::string::npos);
}

int davix_get_metalink_url( Context & c, const Uri & uri,
                            const RequestParams & _params, Uri & metalink){
    DavixError* tmp_err = NULL;
    RequestParams params(_params);
    // don't follow redirect, we need headers
    params.setTransparentRedirectionSupport(false);
    HeadRequest req(c, uri, &tmp_err);
    req.setParameters(params);
    req.addHeaderField("Accept", "application/metalink4+xml");


    DAVIX_TRACE("Executing head query to %s for Metalink file", uri.getString().c_str());
    if(tmp_err != NULL || (req.executeRequest(&tmp_err) <0))
        throw DavixException(davix_scope_meta(), tmp_err->getStatus(), tmp_err->getErrMsg());

    HeaderVec headers;
    req.getAnswerHeaders(headers);
    for(HeaderVec::iterator it = headers.begin(); it != headers.end(); it++){
        if( davix_metalink_header_parser(it->first, it->second, uri, metalink) > 0)
            return 1;

        if(davix_metalink_header_content_type(it->first, it->second)){
            // is a metalink content type, get it
            metalink = uri;
            return 1;
        }
    }




    return 0;
}


int davix_file_get_metalink_to_vfile(Context & c, const Uri & metalink_uri,
                                     const RequestParams & _params, std::vector<DavFile> & vec){
    DavixError * tmp_err=NULL;
    GetRequest req(c, metalink_uri, &tmp_err);
    MetalinkParser parser(c, vec);

    req.setParameters(_params);
    req.addHeaderField("Accept", "application/metalink4+xml");

    DAVIX_TRACE("Executing query for %s Metalink content", metalink_uri.getString().c_str());
    if(tmp_err != NULL || (req.beginRequest(&tmp_err) <0) )
        throw DavixException(davix_scope_meta(), tmp_err->getStatus(), tmp_err->getErrMsg());
    if(httpcodeIsValid(req.getRequestCode()) == false){
        std::ostringstream ss;
        ss << "Unable to get Metalink file, error HTTP " << req.getRequestCode();
        throw DavixException(davix_scope_meta(), StatusCode::InvalidServerResponse, ss.str());
    }

    dav_ssize_t read_size;
    do{
        char buffer[2049];
        buffer[2048] = '\0';
        if( (read_size = req.readSegment(buffer, 2048, &tmp_err)) < 0)
            throw DavixException(davix_scope_meta(), tmp_err->getStatus(), tmp_err->getErrMsg());
        parser.parseChuck(buffer, read_size);
    }while(read_size > 0);

    req.endRequest(NULL);
    return vec.size();
}

void davix_file_get_all_replicas_metalink( Context & c, const Uri & uri,
                                 const RequestParams & _params, std::vector<DavFile> & vec){
    Uri metalink;
    if(davix_get_metalink_url(c,  uri,
                              _params, metalink) > 0
            && davix_file_get_metalink_to_vfile(c, metalink,_params, vec) > 0){
        return;

    }
    throw DavixException(davix_scope_meta(), StatusCode::OperationNonSupported, "Server does not support Metalink standard");
}


MetalinkOps::MetalinkOps()
{

}

MetalinkOps::~MetalinkOps(){

}


std::vector<File> & MetalinkOps::getReplicas(std::vector<File> &vec){
    davix_file_get_all_replicas_metalink(getParams()._context, getParams()._uri, getParams()._reqparams, vec);
    return vec;
}



} // namespace Davix

