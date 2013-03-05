#include "fileproperties.hpp"

namespace Davix {

FileProperties::FileProperties() :
    filename(),
    raw_reps(),
    req_status(0),
    nlink(0),
    uid(0),
    gid(0),
    size(0),
    mode(0),
    atime(0),
    mtime(0),
    ctime(0){

}


} // namespace Davix
