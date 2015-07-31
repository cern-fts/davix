#ifndef S3DELETEPARSER_HPP
#define S3DELETEPARSER_HPP

#include <deque>

#include <davix_internal.hpp>
#include <xml/davxmlparser.hpp>
#include <utils/davix_fileproperties.hpp>
#include <string.h>

namespace Davix{

class S3DeleteParser :  public XMLPropParser
{
public:
    struct Internal;
    S3DeleteParser();

    virtual ~S3DeleteParser();

    virtual std::deque<FileProperties> & getProperties(); // not used
    std::deque<FileDeleteStatus> & getDeleteStatus();


protected:
    virtual int parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts);
    virtual int parserCdataCb(int state, const char *cdata, size_t len);
    virtual int parserEndElemCb(int state, const char *nspace, const char *name);


private:
    Ptr::Scoped<Internal> d_ptr;
};

}

#endif // S3DELETEPARSER_HPP
