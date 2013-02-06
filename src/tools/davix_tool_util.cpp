#include "davix_tool_util.hpp"



namespace Davix{

namespace Tool{


int setup_credential(OptParams & opts, DavixError** err){
    if(opts.cred_path.empty() == false){
        X509Credential cred;
        if( cred.loadFromFilePEM(opts.cred_path, opts.cred_path, "", err) <0){
            return -1;
        }
        opts.params.setClientCertX509(cred);
    }
    return 0;
}

}
}
