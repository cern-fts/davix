#include "davixxmlparserexception.h"

namespace Davix {

DavixXmlParserException::DavixXmlParserException(const Glib::Quark & domain, int code, const Glib::ustring& message) :
            xmlpp::exception(Glib::ustring("Webdav Xml parser Error : ").append(message)),
        q(domain),
        _code(code)
{
}

} // namespace Davix
