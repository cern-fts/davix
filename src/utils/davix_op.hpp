#ifndef DAVIX_OP_HPP
#define DAVIX_OP_HPP

#include <davix.hpp>
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>

namespace Davix{

class DavixOp{

protected:
    std::string _target_url;
    std::string _destination_url;
    std::string opType;
    Tool::OptParams _opts;
    std::string _scope;

    DavixOp(Tool::OptParams opts, std::string target_url, std::string destination_url);
    
    //virtual ~DavixOp();

public:
    virtual int executeOp()=0; 
    virtual ~DavixOp();
    std::string getTargetUrl();
    std::string getDestinationUrl();
    std::string getOpType();

private:

};


class GetOp : public DavixOp{

public:
    GetOp(Tool::OptParams opts, std::string target_url, std::string destination_url);
    virtual ~GetOp();
    virtual int executeOp();
    int getOutFd();

private:

};


class PutOp : public DavixOp{

public:
    PutOp(Tool::OptParams opts, std::string target_url, std::string destination_url);
    virtual ~PutOp();
    virtual int executeOp();
    int getInFd();

private:

    
};
    
}

#endif // DAVIX_OP_HPP
