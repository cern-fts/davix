#ifndef DAVMETA_HPP
#define DAVMETA_HPP

#include <davixcontext.hpp>
#include <params/davixrequestparams.hpp>
#include <file/davfile.hpp>

namespace Davix{


// get all reps from webdav queries
ssize_t webdavGetAllReplicas(Context & c, const Uri & r,
                              const RequestParams & params, ReplicaVec & vec, DavixError** err);


} // Davix




#endif // DAVMETA_HPP
