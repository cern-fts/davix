#ifndef DAVFILE_HPP
#define DAVFILE_HPP

#include <davixcontext.hpp>
#include <params/davixrequestparams.hpp>
#include <davix_types.h>


#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif


namespace Davix{

struct DavFileInternal;


typedef std::vector<Uri> ReplicaVec;

class DavFile
{
public:
    DavFile(Context & c, const Uri & u);
    virtual ~DavFile();

    ///
    /// @brief return all replicas associated to this file
    ///
    /// Replicas are found using a corresponding meta-link file or Webdav extensions if supported
    ///
    /// @param params: Davix Request parameters
    /// @param vec : Replica vector
    /// @param err : DavixError error report
    /// @return  the number of replicas if found, -1 if error.
    ssize_t getAllReplicas(const RequestParams & params, ReplicaVec & vec, DavixError** err);

private:
    DavFileInternal* d_ptr;
    DavFile(const DavFile & f);
    DavFile & operator=(const DavFile & f);

};




} // Davix

#endif // DAVFILE_HPP
