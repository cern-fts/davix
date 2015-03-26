#include "davix_op.hpp"
#include <utils/davix_logger_internal.hpp>

namespace Davix{

//-------------------------------------------------
//----------------------DavixOp--------------------
//-------------------------------------------------
DavixOp::DavixOp(Tool::OptParams opts, std::string target_url, std::string destination_url) :
    _target_url(target_url),
    _destination_url(destination_url),
    _opts(opts),
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
GetOp::GetOp(Tool::OptParams opts, std::string target_url, std::string destination_url) :
    DavixOp(opts, target_url, destination_url)
{
    opType = "GET";
    _scope = "Davix::DavixOp::GetOp";
}

GetOp::~GetOp(){}

int GetOp::executeOp(){
    int ret = -1;
    int fd = -1;
    DavixError* tmp_err=NULL;
    Context c;
    configureContext(c, _opts);
    DavFile f(c, _target_url);

    if((fd = getOutFd())> 0){
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "{} executing op on ", _scope, _target_url);
        ret = f.getToFd(&_opts.params, fd, &tmp_err);
        
        //if getToFd failed, remove the just created blank local file
        if(tmp_err){
            if(tmp_err->getStatus() == StatusCode::FileNotFound)
                ret = -2;
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
PutOp::PutOp(Tool::OptParams opts, std::string target_url, std::string destination_url) :
    DavixOp(opts, target_url, destination_url)
{
    opType = "PUT";
    _scope = "Davix::DavixOp::PutOp";
}

PutOp::~PutOp(){}

int PutOp::executeOp(){
}

int PutOp::getInFd(){
}

} // namespace Davix
