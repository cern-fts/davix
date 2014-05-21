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

}

#endif // METALINKOPS_HPP
