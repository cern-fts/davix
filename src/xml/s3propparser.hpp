#ifndef S3PROPPARSER_HPP
#define S3PROPPARSER_HPP

#include <deque>

#include <davix_internal.hpp>
#include <xml/davxmlparser.hpp>
#include <utils/davix_fileproperties.hpp>
#include <string.h>

namespace Davix{

class S3PropParser :  public XMLPropParser
{
public:
    struct Internal;

    S3PropParser(bool flat_flag);
    S3PropParser(bool flat_flag, std::string fixed_prefix);
    virtual ~S3PropParser();

    virtual std::deque<FileProperties> & getProperties();


protected:
    virtual int parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts);
    virtual int parserCdataCb(int state, const char *cdata, size_t len);
    virtual int parserEndElemCb(int state, const char *nspace, const char *name);


private:
    Ptr::Scoped<Internal> d_ptr;
};

}

#endif // S3PROPPARSER_HPP
