#ifndef METALINKOPS_HPP
#define METALINKOPS_HPP


#include <davixcontext.hpp>
#include <params/davixrequestparams.hpp>
#include <file/davfile.hpp>
#include <fileops/httpiochain.hpp>

namespace Davix{

class MetalinkOps: public HttpIOChain{
public:
    MetalinkOps();
    virtual ~MetalinkOps();

    // calc replica
    virtual std::vector<File> & getReplicas(std::vector<File> & vec);
};

// utilities
int davix_metalink_header_parser(const std::string & header_key, const std::string & header_value,
                                 const Uri & u_original,
                                 Uri & metalink);


}

#endif // METALINKOPS_HPP
