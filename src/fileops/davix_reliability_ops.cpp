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

#include <mutex>
#include <davix_internal.hpp>
#include "davix_reliability_ops.hpp"

#include <utils/stringutils.hpp>
#include <utils/davix_logger_internal.hpp>
#include <xml/metalinkparser.hpp>
#include "libs/alibxx/crypto/base64.hpp"


namespace Davix{


using namespace StrUtil;

typedef std::function< dav_ssize_t (IOChainContext &)> FuncIO;
typedef std::function< StatInfo & (IOChainContext &) > FuncStatInfo;


static bool metalink_support_disabled=false;
static std::once_flag metalink_once;


void propagateNonRecoverableExceptions(DavixException & e){
    /// Forward redirections and other error we don't want to recover
    if(e.code() == StatusCode::RedirectionNeeded
            || e.code() == StatusCode::OperationTimeout){
        throw e;
    }
}

void metalink_check(){
    metalink_support_disabled = (getenv("DAVIX_DISABLE_METALINK") != NULL);
}

static bool isMetalinkDisabled(const RequestParams* params){
    std::call_once(metalink_once, metalink_check);
    return (params != NULL && params->getMetalinkMode() == MetalinkMode::Disable) || metalink_support_disabled;
}


template<class Executor, class ReturnType>
ReturnType metalinkTryReplicas(HttpIOChain & chain, IOChainContext & io_context, Executor fun){
    std::vector<File> replicas;

    // check if we expired
    io_context.checkTimeout();
    // get all replicas from Metalink
    chain.getReplicas(io_context, replicas);
    for(std::vector<File>::iterator it = replicas.begin();it != replicas.end(); ++it){
        IOChainContext internal_context(io_context._context, it->getUri(), io_context._reqparams);
        internal_context.fdHandler = io_context.fdHandler;

        try{
            return fun(internal_context);
        }catch(DavixException & replica_error){
            DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_CHAIN, "Fail access to replica {}: {}", it->getUri(), replica_error.what());
        }catch(...){
            DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_CHAIN, "Fail access to replica: Unknown Error");
        }

        io_context.fdHandler = internal_context.fdHandler;
        // check timeout again between two iterations
        io_context.checkTimeout();
    }
    throw DavixException(davix_scope_io_buff(), StatusCode::InvalidServerResponse, "Impossible to access any of the replicas with success");
}


template<class Executor, class ReturnType>
ReturnType metalinkExecutor(HttpIOChain & chain, IOChainContext & io_context, Executor fun){
    // if disabled, do nothing
    if(isMetalinkDisabled(io_context._reqparams)){
        return fun(io_context);
    }

    try{
        // Execute operation
        return fun(io_context);
    }catch(DavixException & e){

        propagateNonRecoverableExceptions(e);

        DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_CHAIN, "Could not execute operation on {}, error {}", io_context._uri.getString(), e.what());
        DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_CHAIN, "Try to Recover with Metalink...");

        try{
            return metalinkTryReplicas<Executor, ReturnType>(chain, io_context, fun);
        }catch(DavixException & metalink_error){
            DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_CHAIN, "Impossible to Recover with Metalink: {}", metalink_error.what());
        }catch(...){
            DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_CHAIN, "Impossible to Recover with Metalink: Unknown Error");
        }
        throw e;
    }
}



int davix_metalink_header_parser(const std::string & header_key, const std::string & header_value,
                                 const Uri & u_original,
                                 Uri & metalink){
    DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CHAIN, "Parse headers for metalink {} {}", header_key, header_value);

    if(compare_ncase(header_key, "Link") ==0 && header_value.find("application/metalink") != std::string::npos){
        std::string::const_iterator it1, it2;
        if( ( it1 = std::find(header_value.begin(), header_value.end(), '<')) != header_value.end()
                && ( it2 = std::find(it1, header_value.end(), '>')) != header_value.end()){
            std::string metalink_url(it1+1, it2);
            metalink =  Uri::fromRelativePath(u_original, trim(metalink_url));
            if(metalink.getStatus() == StatusCode::OK){
                DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CHAIN, "Valid metalink URI found {}", metalink.getString());
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


    DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CHAIN, "Executing head query to {} for Metalink file", uri.getString());


    if(tmp_err != NULL || (req.executeRequest(&tmp_err) <0)) {
      if (tmp_err)
        throw DavixException(davix_scope_meta(), tmp_err->getStatus(), tmp_err->getErrMsg());
      else
        throw DavixException(davix_scope_meta(), Davix::StatusCode::UnknownError, "Unknown error");
    }

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

    DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CHAIN, "Executing query for {} Metalink content", metalink_uri.getString());
    if(tmp_err != NULL || (req.beginRequest(&tmp_err) <0) )
        throw DavixException(davix_scope_meta(), tmp_err->getStatus(), tmp_err->getErrMsg());
    if(httpcodeIsValid(req.getRequestCode()) == false){
        throw DavixException(davix_scope_meta(), StatusCode::InvalidServerResponse, fmt::format("Unable to get Metalink file, error HTTP {}", req.getRequestCode()));
    }

    dav_ssize_t read_size;
    do{
        char buffer[2049];
        buffer[2048] = '\0';
        if( (read_size = req.readSegment(buffer, 2048, &tmp_err)) < 0)
            throw DavixException(davix_scope_meta(), tmp_err->getStatus(), tmp_err->getErrMsg());
        parser.parseChunk(buffer, read_size);
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



std::vector<File> & MetalinkOps::getReplicas(IOChainContext & iocontext, std::vector<File> &vec){
    davix_file_get_all_replicas_metalink(iocontext._context, iocontext._uri, iocontext._reqparams, vec);
    return vec;
}

StatInfo & MetalinkOps::statInfo(IOChainContext &iocontext, StatInfo &st_info){
    FuncStatInfo func(std::bind(&HttpIOChain::statInfo, _next.get(), std::placeholders::_1, std::ref(st_info)));
    return metalinkExecutor<FuncStatInfo, StatInfo &>(*this, iocontext, func);
}

dav_ssize_t MetalinkOps::read(IOChainContext &iocontext, void *buf, dav_size_t count){
    FuncIO func(std::bind(&HttpIOChain::read, _next.get(), std::placeholders::_1, buf, count));
    return metalinkExecutor<FuncIO, dav_ssize_t>(*this, iocontext, func);
}

dav_ssize_t MetalinkOps::pread(IOChainContext &iocontext, void *buf, dav_size_t count, dav_off_t offset){

    FuncIO func(std::bind(&HttpIOChain::pread, _next.get(), std::placeholders::_1, buf, count, offset));
    return metalinkExecutor<FuncIO, dav_ssize_t>(*this, iocontext, func);
}


dav_ssize_t MetalinkOps::preadVec(IOChainContext & iocontext, const DavIOVecInput * input_vec,
                          DavIOVecOuput * output_vec,
                          const dav_size_t count_vec){

    FuncIO func(std::bind(&HttpIOChain::preadVec, _next.get(), std::placeholders::_1, input_vec, output_vec, count_vec));
    return metalinkExecutor<FuncIO, dav_ssize_t>(*this, iocontext, func);
}

// read to fd Metalink manager
dav_ssize_t MetalinkOps::readToFd(IOChainContext & iocontext, int fd, dav_size_t size){
    FuncIO func(std::bind(&HttpIOChain::readToFd, _next.get(), std::placeholders::_1, fd, size));
    return metalinkExecutor<FuncIO, dav_ssize_t>(*this, iocontext, func);
}











///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

template<class Executor, class ReturnType>
ReturnType autoRetryExecutor(HttpIOChain & chain, IOChainContext & io_context, Executor fun){

    (void) chain;
    const int max_retry = io_context._reqparams->getOperationRetry();
    const int retry_delay = io_context._reqparams->getOperationRetryDelay();
    int retry =1;
    const Uri & u = io_context._uri;

     while(1){
        io_context.checkTimeout();
        try{
            return fun(io_context);
        }catch(DavixException & error){

            // propagate fatal exceptions and connection error exceptions
            propagateNonRecoverableExceptions(error);
            // we can not recover from connexion timeout
            // throw exception, cancel IO chain request
            if(error.code() == StatusCode::ConnectionTimeout){
                throw error;
            }
            // we should also just give up if the server responds with 403 or 405
            else if(error.code() == StatusCode::PermissionRefused){
                throw error;
            }

            DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_CHAIN, "Negative result for operation: {}. After {} retry", error.what(), retry);
            if( retry >= max_retry){
                throw DavixException(error.scope(), error.code(), fmt::format("Result {} after {} attempts", error.what(), retry));
            }
        }catch(...){
            DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_CHAIN, "Operation failure: Unknown Error");
            throw DavixException(davix_scope_io_buff(), StatusCode::UnknownError, fmt::format("Unrecoverable error from IOChain on {}", u));
        }
        ++retry;
        sleep(retry_delay);
    }
}


AutoRetryOps::AutoRetryOps(){

}


AutoRetryOps::~AutoRetryOps(){

}


StatInfo & AutoRetryOps::statInfo(IOChainContext &iocontext, StatInfo &st_info){
    FuncStatInfo func(std::bind(&HttpIOChain::statInfo, _next.get(), std::placeholders::_1, std::ref(st_info)));
    return autoRetryExecutor<FuncStatInfo, StatInfo &>(*this, iocontext, func);
}

dav_ssize_t AutoRetryOps::read(IOChainContext &iocontext, void *buf, dav_size_t count){
    FuncIO func(std::bind(&HttpIOChain::read, _next.get(), std::placeholders::_1, buf, count));
    return autoRetryExecutor<FuncIO, dav_ssize_t>(*this, iocontext, func);
}

dav_ssize_t AutoRetryOps::pread(IOChainContext &iocontext, void *buf, dav_size_t count, dav_off_t offset){

    FuncIO func(std::bind(&HttpIOChain::pread, _next.get(), std::placeholders::_1, buf, count, offset));
    return autoRetryExecutor<FuncIO, dav_ssize_t>(*this, iocontext, func);
}


dav_ssize_t AutoRetryOps::preadVec(IOChainContext & iocontext, const DavIOVecInput * input_vec,
                          DavIOVecOuput * output_vec,
                          const dav_size_t count_vec){

    FuncIO func(std::bind(&HttpIOChain::preadVec, _next.get(), std::placeholders::_1, input_vec, output_vec, count_vec));
    return autoRetryExecutor<FuncIO, dav_ssize_t>(*this, iocontext, func);
}

// read to fd Metalink manager
dav_ssize_t AutoRetryOps::readToFd(IOChainContext & iocontext, int fd, dav_size_t size){
    FuncIO func(std::bind(&HttpIOChain::readToFd, _next.get(), std::placeholders::_1, fd, size));
    return autoRetryExecutor<FuncIO, dav_ssize_t>(*this, iocontext, func);
}


} // namespace Davix
