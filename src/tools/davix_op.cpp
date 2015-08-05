/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013  
 * Author: Kwong Tat Cheung <kwong.tat.cheung@cern.ch>
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

#include "davix_op.hpp"
#include <utils/davix_logger_internal.hpp>
#include <sstream>
#include <xml/s3deleteparser.hpp>
#include <xml/davdeletexmlparser.hpp>

namespace Davix{

//-------------------------------------------------
//----------------------DavixOp--------------------
//-------------------------------------------------
DavixOp::DavixOp(const Tool::OptParams& opts, std::string target_url, std::string destination_url, Context& c) :
    _target_url(target_url),
    _destination_url(destination_url),
    _opts(opts),
    _c(c),
    _scope()
{
}

DavixOp::~DavixOp(){}

std::string DavixOp::getTargetUrl(){
    return _target_url;
}

std::string DavixOp::getDestinationUrl(){
    return _destination_url;
}

std::string DavixOp::getOpType(){
    return opType;
}


//-------------------------------------------------
//----------------------GetOp----------------------
//-------------------------------------------------
GetOp::GetOp(const Tool::OptParams& opts, std::string target_url, std::string destination_url, Context& c) :
    DavixOp(opts, target_url, destination_url, c)
{
    opType = "GET";
    _scope = "Davix::DavixOp::GetOp";
}

GetOp::~GetOp(){}

int GetOp::executeOp(){
    int ret = -1;
    int fd = -1;
    DavixError* tmp_err=NULL;
    DavFile f(_c, _target_url);

    if((fd = getOutFd())> 0){
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "{} executing op on ", _scope, _target_url);
        ret = f.getToFd(&_opts.params, fd, &tmp_err);
        
        //if getToFd failed, remove the just created blank local file
        if(tmp_err){
            std::cerr << std::endl << _scope << " Failed to GET " << _target_url << std::endl;
            Tool::errorPrint(&tmp_err);
            remove(_destination_url.c_str());
        }
        close(fd);
    }
    return ret;
}

int GetOp::getOutFd(){
    DavixError* tmp_err=NULL;
    int fd = -1;
    if(_destination_url.empty() == false){
        // for S3 we have to split the key into tokens and create a dir for each token
        if (_opts.params.getProtocol() == RequestProtocol::AwsS3){
            if((Tool::mkdirP(_destination_url, true)) < 0 ){
                std::cerr << std::endl << _scope << " Failed to create local directory for " << _destination_url << std::endl;
                return -1;
            }
        }
        if((fd = open(_destination_url.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0777)) <0  ){
            davix_errno_to_davix_error(errno, _scope, std::string("while opening/creating ").append(_destination_url), &tmp_err);
            if(tmp_err){
                Tool::errorPrint(&tmp_err);
            }
            return -1;
        }

    }else{
        fd = dup(STDOUT_FILENO);
    }
    return fd;    
}


//-------------------------------------------------
//----------------------PutOp----------------------
//-------------------------------------------------
PutOp::PutOp(const Tool::OptParams& opts, std::string target_url, std::string destination_url, dav_size_t file_size, Context& c) :
    DavixOp(opts, target_url, destination_url, c)
{
    opType = "PUT";
    _scope = "Davix::DavixOp::PutOp";
    _file_size = file_size;

}

PutOp::~PutOp(){}

int PutOp::executeOp(){
    DavixError* tmp_err=NULL;
    int fd = -1;

    if( (fd = getInFd(&tmp_err)) < 0){
        if(tmp_err)
            Tool::errorPrint(&tmp_err);
        return -1;
    }
    
    TRY_DAVIX{
        DavFile f(_c, _destination_url);
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "{} executing op on ", _scope, _destination_url);
        f.put(&_opts.params, fd, _file_size);
        close(fd);
        return 0;
    }CATCH_DAVIX(&tmp_err);

    if(fd != -1)
        close(fd);

    if(tmp_err->getStatus() == StatusCode::FileExist){
        std::cerr << std::endl << _scope << " " << _destination_url << " already exists, continuing..." << std::endl;
    }
    else
        std::cerr << std::endl << _scope << " Failed to PUT " << _target_url << std::endl;

    Tool::errorPrint(&tmp_err);
    return -1;
}

int PutOp::getInFd(DavixError** err){
    int fd = -1;
    if(_target_url.empty() == false){
        if((fd = open(_target_url.c_str(), O_RDONLY)) <0  ){
            davix_errno_to_davix_error(errno, _scope, std::string("for source file ").append(_target_url), err);
            return -1;
        }
    }else{
        fd = dup(STDOUT_FILENO);
    }
    return fd;
    
}


//-------------------------------------------------
//--------------------DeleteOp---------------------
//-------------------------------------------------
DeleteOp::DeleteOp(const Tool::OptParams& opts, std::string destination_url, Context& c, std::string buf) :
    DavixOp(opts, "", destination_url, c)
{
    opType = "DELETE";
    _scope = "Davix::DavixOp::DeleteOp";
    _buf = buf;
}

DeleteOp::~DeleteOp(){}

int DeleteOp::executeOp(){
    DavixError* tmp_err=NULL;

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "{} executing op on ", _scope, _destination_url);

    if(_opts.params.getProtocol() == RequestProtocol::AwsS3){
        _destination_url += "/?delete";
        PostRequest req(_c, _destination_url, &tmp_err);

        if(tmp_err){
            Tool::errorPrint(&tmp_err);
            return -1;
        }

        req.setParameters(_opts.params);
        
        std::ostringstream ss;
        ss << _buf.size();

        // calculate md5 of body and set header fields, these are required for S3 multi-objects delete
        std::string md5;
        S3::calculateMD5(_buf, md5);

        req.addHeaderField("Content-MD5", md5);
        req.addHeaderField("Content-Length", ss.str());
              
        req.setRequestBody(_buf);

        req.executeRequest(&tmp_err);

        if(tmp_err){
            Tool::errorPrint(&tmp_err);
            return -1;
        }

        // check response code
        int code = req.getRequestCode();

        if(!httpcodeIsValid(code)){
            httpcodeToDavixError(req.getRequestCode(), davix_scope_http_request(), "during S3 multi-objects delete operation", &tmp_err);
            if(tmp_err){
                Tool::errorPrint(&tmp_err);
                return -1;
            }
        }

        std::vector<char> body = req.getAnswerContentVec();

        TRY_DAVIX{
            parse_deletion_result(code, Uri(_destination_url), _scope, body);
        }CATCH_DAVIX(&tmp_err);

        if(tmp_err){
            Tool::errorPrint(&tmp_err);
            return -1;
        }
    }
    else{
        // cases other than s3, not implenmented for now. WebDAV delete collection already works without the -r switch
        return -1;
    }
    
    return -1;
}

void DeleteOp::parse_deletion_result(int code, const Uri & u, const std::string & scope, const std::vector<char> & body){
    switch(code){
        case 200:{
            // if s3 && scope was davix_scope_rm_str() && batch delete, parse body
            S3DeleteParser parser;
            parser.parseChunk(&(body[0]), body.size());

            // check if any of the delete request entry is flagged as error, if so, just print them for now
            if( parser.getDeleteStatus().size() > 0){
                for(unsigned int i=0; i < parser.getDeleteStatus().size(); ++i){
                    if(parser.getDeleteStatus().at(i).error){
                        std::ostringstream ss;
                        ss << "Error: " << parser.getDeleteStatus().at(i).error_code << 
                            " -> " << parser.getDeleteStatus().at(i).message << 
                            " encountered while atempting to delete " << 
                            parser.getDeleteStatus().at(i).filename;

                        std::cerr << std::endl << ss.str() << std::endl;
                    }
                    return;
                }
                // if no properties, status were filtered because invalid
                httpcodeToDavixException(404, scope);
                return;
            }
        }
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


} // namespace Davix
