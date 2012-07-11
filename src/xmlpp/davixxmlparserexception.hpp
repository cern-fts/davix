#ifndef DAVIX_DAVIXXMLPARSEREXCEPTION_H
#define DAVIX_DAVIXXMLPARSEREXCEPTION_H

#include <libxml++/libxml++.h>
#include <glibmm/quark.h>

namespace Davix {

class DavixXmlParserException : public xmlpp::exception
{
public:
    DavixXmlParserException(const Glib::Quark & domain, int code, const Glib::ustring& message);
    virtual ~DavixXmlParserException() throw (){} ;

protected:
    Glib::Quark q;
    int _code;

};

} // namespace Davix

#endif // DAVIX_DAVIXXMLPARSEREXCEPTION_H
