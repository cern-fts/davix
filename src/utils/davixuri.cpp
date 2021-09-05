/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#include <davix_internal.hpp>
#include <utils/davix_uri.hpp>


namespace Davix {

/**
 * @cond HIDDEN_SYMBOLS
 */

const std::string void_str("");

int davix_uri_parse(const std::string & uri_str, UriPrivate & res);
int davix_uri_cmp(const UriPrivate &u1, const UriPrivate &u2);
std::string davix_path_escape(const std::string & str, bool escapeSlashes);
std::string davix_path_unescape(const std::string & str);

struct UriPrivate{
    UriPrivate() :
        code(StatusCode::UriParsingError),
        proto(),
        userinfo(),
        path(),
        host(),
        query(),
        fragment(),
        port(0),
        _uri_string() {}

    UriPrivate(const UriPrivate & orig):
        code(orig.code),
        proto(orig.proto),
        userinfo(orig.userinfo),
        path(orig.path),
        host(orig.host),
        query(orig.query),
        fragment(orig.fragment),
        port(orig.port),
        _uri_string(orig._uri_string) {}

    ~UriPrivate(){

    }

    void parsing(const std::string & uri_string){
        _uri_string = uri_string;
        if( davix_uri_parse(_uri_string, *this) == 0
             && proto.size() != 0
             && port != UINT_MAX){
            code = StatusCode::OK;
        }else{
            clear();
        }
    }

    void addParam(const std::string & key, const std::string & value, std::string & target) {
        if(target.size() == 0) {
            target = key;
            target.append("=");
            target.append(value);
        }
        else {
            target.append("&");
            target.append(key);
            target.append("=");
            target.append(value);
        }
        _update_string();
    }

    void addQueryParam(const std::string & key, const std::string & value) {
        addParam(key, value, query);
    }

    void addFragmentParam(const std::string & key, const std::string & value) {
        addParam(key, value, fragment);
    }

    void addPathSegment(const std::string &seg) {
        ensureTrailingSlash();
        path += seg;
        _update_string();
    }

    void ensureTrailingSlash() {
        if(path.size() == 0 || path[path.size() - 1] != '/') {
            path += "/";
            _update_string();
        }
    }

    void removeTrailingSlash() {
        if(path.size() != 0 && path[path.size() - 1] == '/') {
            path.erase(path.size()-1, 1);
            _update_string();
        }
    }

    void clear(){
        proto.clear();
        userinfo.clear();
        path.clear();
        host.clear();
        query.clear();
        fragment.clear();
        port = 0;
    }

    void _update_string() {
        std::ostringstream ss;
        ss << proto << "://";
        if(userinfo.size() > 0) ss << "@" << userinfo;
        ss << host;
        if(port != 0) ss << ":" << port;
        ss << path;
        if(query.size() > 0) ss << "?" << query;
        if(fragment.size() > 0) ss << "#" << fragment;

        _uri_string = ss.str();
    }

    StatusCode::Code code;
    std::string proto, userinfo, path, host, query, fragment;
    unsigned int port;
    std::string _uri_string;
    std::unique_ptr<std::string> query_and_path;

};

Uri::Uri() :
    d_ptr(new UriPrivate()){
}


Uri::Uri(const std::string & uri) :
    d_ptr(new UriPrivate()){
    d_ptr->parsing(uri);
}

Uri::Uri(const Uri & uri) :
    d_ptr(new UriPrivate(*(uri.d_ptr))){
}

void Uri::addQueryParam(const std::string & key, const std::string & value) {
    d_ptr->addQueryParam(Uri::queryParamEscape(key), Uri::queryParamEscape(value));
}


void Uri::addFragmentParam(const std::string & key, const std::string & value) {
    d_ptr->addFragmentParam(key, value);
}

void Uri::addPathSegment(const std::string &seg) {
    d_ptr->addPathSegment(seg);
}

void Uri::ensureTrailingSlash() {
    d_ptr->ensureTrailingSlash();
}

void Uri::removeTrailingSlash() {
    d_ptr->removeTrailingSlash();
}

bool Uri::queryParamExists(const std::string &key) const {
    ParamVec queryVec = this->getQueryVec();
    for(ParamVec::iterator it = queryVec.begin(); it != queryVec.end(); it++) {
        if(it->first == key)
            return true;
    }
    return false;
}

Uri::~Uri(){
    delete d_ptr;
}

int Uri::getPort() const{
    if(d_ptr->code != StatusCode::OK)
        return -1;
    return static_cast<int>(d_ptr->port);
}

const std::string & Uri::getHost() const{
    return d_ptr->host;
}

const std::string & Uri::getString() const{
    return d_ptr->_uri_string;
}

const std::string & Uri::getProtocol() const {
    return d_ptr->proto;
}

const std::string & Uri::getUserInfo() const{
    return d_ptr->userinfo;
}

const std::string & Uri::getPath() const {
    return d_ptr->path;
}

void Uri::setPath(const std::string & path) {
    d_ptr->path = path;
    d_ptr->_update_string();
}

void Uri::setProtocol(const std::string & protocol) {
    d_ptr->proto = protocol;
    d_ptr->_update_string();
}

void Uri::httpizeProtocol() {
    if(d_ptr->proto == "s3" || d_ptr->proto == "dav" || d_ptr->proto == "gcloud" || d_ptr->proto == "swift" || d_ptr->proto == "cs3") {
        setProtocol("http");
    }
    if(d_ptr->proto == "s3s" || d_ptr->proto == "davs" || d_ptr->proto == "gclouds" || d_ptr->proto == "swifts" || d_ptr->proto == "cs3s") {
        setProtocol("https");
    }
}

const std::string & Uri::getPathAndQuery() const {
    if(d_ptr->query_and_path.get() == NULL){
        if(d_ptr->query.size() > 0){
            d_ptr->query_and_path.reset(new std::string(d_ptr->path + "?" + d_ptr->query));
        }else{
            d_ptr->query_and_path.reset(new std::string(d_ptr->path));
        }
    }
    return *(d_ptr->query_and_path);
}

static ParamVec splitParams(const std::string &str) {
    ParamVec params;
    const std::string paramSeparator = "&";
    const std::string pairSeparator = "=";

    std::string::size_type curr = 0;
    while(curr < str.length()) {
        std::string::size_type next = str.find(paramSeparator, curr);
        if(next == std::string::npos) next = str.length();
        std::string param = str.substr(curr, next-curr);

        // separate the two parts between '='
        std::string::size_type eq = param.find(pairSeparator);
        if(eq == std::string::npos) {
            params.push_back(ParamLine(param, ""));
        } else {
            params.push_back(ParamLine(param.substr(0, eq), param.substr(eq+1, param.length())));
        }

        curr = next+1;
    }
    return params;
}

const std::string & Uri::getFragment() const {
    return d_ptr->fragment;
}

bool Uri::fragmentParamExists(const std::string &param) const {
    ParamVec fragmentVec = splitParams(getFragment());
    for(ParamVec::iterator it = fragmentVec.begin(); it != fragmentVec.end(); it++) {
        if(it->first == param)
            return true;
    }
    return false;
}

const std::string Uri::getFragmentParam(const std::string &param) const {
    ParamVec fragmentVec = splitParams(getFragment());
    for(ParamVec::iterator it = fragmentVec.begin(); it != fragmentVec.end(); it++) {
        if(it->first == param)
            return it->second;
    }
    return "";
}

ParamVec Uri::getQueryVec() const {
    return splitParams(getQuery());
}

const std::string & Uri::getQuery() const{
    return d_ptr->query;
}

Uri & Uri::operator =(const Uri & orig){
    if(this == &orig)
        return *this;

    if (d_ptr) delete d_ptr;
    d_ptr = new UriPrivate(*(orig.d_ptr));
    return *this;
}

StatusCode::Code Uri::getStatus() const{
    return d_ptr->code;
}


bool Uri::equal(const Uri &u2) const{
    if(this->getStatus() != Davix::StatusCode::OK || u2.getStatus() != Davix::StatusCode::OK)
        return false;

    return davix_uri_cmp(*d_ptr, *(u2.d_ptr))==0;
}

bool Uri::operator ==(const Uri & u2) const{
    return this->equal(u2);
}


std::string Uri::escapeString(const std::string & str){
    return davix_path_escape(str, false);
}

std::string Uri::unescapeString(const std::string & str){
    return davix_path_unescape(str);
}

std::string Uri::queryParamEscape(const std::string & str) {
    return davix_path_escape(str, true);
}

// Determine whether this is a URL, and if so, URI-escape the right part.
// Otherwise, simple string concatenation.
std::string Uri::join(const std::string &left, const std::string &right) {
    Uri leftUri(left);

    if(leftUri.getStatus() != StatusCode::OK) {
        if(left.size() == 0 || left[left.size()-1] != '/') {
            return left + "/" + right;
        }
        return left + right;
    }

    leftUri.ensureTrailingSlash();
    return leftUri.getString() + Uri::escapeString(right);
}



// FIX IT : does not manage properly ../
Uri Uri::fromRelativePath(const Uri &uri, const std::string &relPath){
    std::ostringstream ss;
    if(relPath.size() >= 2){
        // test if not absolute
        std::string::const_iterator it;
        if( ( it = std::find(relPath.begin(), relPath.end(), '/')) != relPath.end()
                && it != relPath.begin() && *(it-1) == ':'
                && (it+1) != relPath.end() && *(it+1) == '/'){
            return Uri(relPath);
        }

        // RFC 3986 network-path reference”
        if(relPath[0] == '/' && relPath[1] == '/'){

            ss << uri.getProtocol() << ":" << relPath;
            return Uri(ss.str());
        }

        // RFC 3986 relative ”
        if(relPath[0] == '.' && relPath[1] == '/'){
            ss << uri.getString() << "/";
            std::copy(relPath.begin()+2,relPath.end(), std::ostreambuf_iterator<char>(ss));
            return Uri(ss.str());
        }
    }

    // RFC 3986 abs path ”
    if( relPath.size() >= 1){
        if(relPath[0] == '/'){
            ss << uri.getProtocol() << "://";
            if(uri.getUserInfo().size() >0){
                ss << uri.getUserInfo() << '@';
            }
            ss << uri.getHost();
            if(uri.getPort() != 0){
                ss << ':' << uri.getPort();
            }
            ss << relPath;
            return Uri(ss.str());
        }

    }

    ss << uri.getString() <<'/'<< relPath;
    return Uri(ss.str());
}

bool uriCheckError(const Uri &uri, DavixError **err){
    if(uri.getStatus() == StatusCode::OK)
        return true;
    DavixError::setupError(err, davix_scope_uri_parser(), uri.getStatus(), std::string("Uri syntax Invalid : ").append(uri.getString()));
    return false;
}



unsigned int httpUriGetPort(const Uri & uri){
    int port = uri.getPort();
    if( port ==0){
        const std::string & scheme = uri.getProtocol();
        if(*scheme.rbegin() == 's') // davs, https or s3s
            port = 443;
        else
            port =80;
    }
    return port;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///   part imported from neon parser ne_uri,  converted C++
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/* URI ABNF from RFC 3986: */

#define PS (0x0001) /* "+" */
#define PC (0x0002) /* "%" */
#define DS (0x0004) /* "-" */
#define DT (0x0008) /* "." */
#define US (0x0010) /* "_" */
#define TD (0x0020) /* "~" */
#define FS (0x0040) /* "/" */
#define CL (0x0080) /* ":" */
#define AT (0x0100) /* "@" */
#define QU (0x0200) /* "?" */

#define DG (0x0400) /* DIGIT */
#define AL (0x0800) /* ALPHA */

#define GD (0x1000) /* gen-delims    = "#" / "[" / "]"
                     * ... except ":", "/", "@", and "?" */

#define SD (0x2000) /* sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
                     *               / "*" / "+" / "," / ";" / "="
                     * ... except "+" which is PS */

#define OT (0x4000) /* others */

#define URI_ALPHA (AL)
#define URI_DIGIT (DG)

/* unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~" */
#define URI_UNRESERVED (AL | DG | DS | DT | US | TD)
/* scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." ) */
#define URI_SCHEME (AL | DG | PS | DS | DT)
/* real sub-delims definition, including "+" */
#define URI_SUBDELIM (PS | SD)
/* real gen-delims definition, including ":", "/", "@" and "?" */
#define URI_GENDELIM (GD | CL | FS | AT | QU)
/* userinfo = *( unreserved / pct-encoded / sub-delims / ":" ) */
#define URI_USERINFO (URI_UNRESERVED | PC | URI_SUBDELIM | CL)
/* pchar = unreserved / pct-encoded / sub-delims / ":" / "@" */
#define URI_PCHAR (URI_UNRESERVED | PC | URI_SUBDELIM | CL | AT)
/* invented: segchar = pchar / "/" */
#define URI_SEGCHAR (URI_PCHAR | FS)
/* query = *( pchar / "/" / "?" ) */
#define URI_QUERY (URI_PCHAR | FS | QU)
/* fragment == query */
#define URI_FRAGMENT URI_QUERY

/* any characters which should be path-escaped: */
#define URI_ESCAPE ((URI_GENDELIM & ~(FS)) | URI_SUBDELIM | OT | PC)

static const unsigned int uri_chars[256] = {
/* 0xXX    x0      x2      x4      x6      x8      xA      xC      xE     */
/*   0x */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT,
/*   1x */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT,
/*   2x */ OT, SD, OT, GD, SD, PC, SD, SD, SD, SD, SD, PS, SD, DS, DT, FS,
/*   3x */ DG, DG, DG, DG, DG, DG, DG, DG, DG, DG, CL, SD, OT, SD, OT, QU,
/*   4x */ AT, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL,
/*   5x */ AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, GD, OT, GD, OT, US,
/*   6x */ OT, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL,
/*   7x */ AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, OT, OT, OT, TD, OT,
/*   8x */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT,
/*   9x */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT,
/*   Ax */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT,
/*   Bx */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT,
/*   Cx */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT,
/*   Dx */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT,
/*   Ex */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT,
/*   Fx */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT
};

#define uri_lookup(ch) (uri_chars[(unsigned char)ch])


int davix_uri_parse(const std::string & uri_str, UriPrivate & res)
{
    const char *p, *s, *uri;
    p = s = uri = uri_str.c_str();

    /* => s = p = URI-reference */

    if (uri_lookup(*p) & URI_ALPHA) {
        while (uri_lookup(*p) & URI_SCHEME)
            p++;

        if (*p == ':') {
            res.proto.assign(uri, p -s);
            s = p + 1;
        }
    }

    /* => s = heir-part, or s = relative-part */

    if (s[0] == '/' && s[1] == '/') {
        const char *pa;

        /* => s = "//" authority path-abempty (from expansion of
         * either heir-part of relative-part)  */

        /* authority = [ userinfo "@" ] host [ ":" port ] */

        s = pa = s + 2; /* => s = authority */

        while (*pa != '/' && *pa != '\0')
            pa++;
        /* => pa = path-abempty */

        p = s;
        while (p < pa && uri_lookup(*p) & URI_USERINFO)
            p++;

        if (*p == '@') {
            res.userinfo.assign(s, p-s);
            s = p + 1;
        }
        /* => s = host */

        if (s[0] == '[') {
            p = s + 1;

            while (*p != ']' && p < pa)
                p++;

            if (p == pa || (p + 1 != pa && p[1] != ':')) {
                /* Ill-formed IP-literal. */
                return -1;
            }

            p++; /* => p = colon */
        } else {
            /* Find the colon. */
            p = pa;
            while (*p != ':' && p > s)
                p--;
        }

        if (p == s) {
            p = pa;
            /* No colon; => p = path-abempty */
        } else if (p + 1 != pa) {
            /* => p = colon */
            if(*p == ':') {
              res.port = atoi(p + 1);
              if( res.port  == 0)
                res.port  = UINT_MAX;
            }
        }
        res.host.assign(s, p - s);

        s = pa;

        if (*s == '\0') {
            s = "/"; /* FIXME: scheme-specific. */
        }
    }

    /* => s = path-abempty / path-absolute / path-rootless
     *      / path-empty / path-noscheme */

    p = s;

    while (uri_lookup(*p) & URI_SEGCHAR)
        p++;

    /* => p = [ "?" query ] [ "#" fragment ] */

    res.path.assign(s, p - s);

    if (*p != '\0') {
        s = p++;

        while (uri_lookup(*p) & URI_QUERY)
            p++;

        /* => p = [ "#" fragment ] */
        /* => s = [ "?" query ] [ "#" fragment ] */

        if (*s == '?') {
            res.query.assign(s + 1, p - s - 1);

            if (*p != '\0') {
                s = p++;

                while (uri_lookup(*p) & URI_FRAGMENT)
                    p++;
            }
        }

        /* => p now points to the next character after the
         * URI-reference; which should be the NUL byte. */

        if (*s == '#') {
            res.fragment.assign(s + 1, p - s - 1);
        }
        else if (*p || *s != '?') {
            return -1;
        }
    }

    return 0;
}


// As specified by RFC 2616, section 3.2.3.
int davix_uri_cmp(const UriPrivate & u1, const UriPrivate & u2)
{

    int diff;
    if( (diff= u1.path.compare(u2.path)) != 0)
        return diff;
    if( (diff= strcasecmp(u1.host.c_str(), u2.host.c_str())) != 0)
        return diff;
    if( (diff= strcasecmp(u1.proto.c_str(), u2.proto.c_str())) != 0)
        return diff;
    if( (diff= u1.query.compare(u2.query)) != 0)
        return diff;
    if( (diff= u1.fragment.compare(u2.fragment)) != 0)
        return diff;
    if( (diff= u1.userinfo.compare(u2.userinfo)) != 0)
        return diff;
    return u2.port - u1.port;
}



std::string davix_path_unescape(const std::string & str)
{
    const char *pnt, *uri = (const  char*)str.c_str();
    char *retpos, buf[5] = { "0x00" };

    char buffer[str.size() + 1];
    retpos = buffer;
    for (pnt = uri; *pnt != '\0'; pnt++) {
    if (*pnt == '%') {
        if (!isxdigit((unsigned char) pnt[1]) ||
        !isxdigit((unsigned char) pnt[2])) {
        /* Invalid URI */
                return std::string();
        }
        buf[2] = *++pnt; buf[3] = *++pnt; /* bit faster than memcpy */
        *retpos++ = (char)strtol(buf, NULL, 16);
    } else {
        *retpos++ = *pnt;
    }
    }
    *retpos = '\0';
    return std::string(buffer);
}

/* CH must be an unsigned char; evaluates to 1 if CH should be
 * percent-encoded. */
inline bool path_escape_ch(const char ch, bool escapeSlashes) {
    return (uri_lookup(ch) & URI_ESCAPE) || (escapeSlashes && ch == '/');
}

std::string davix_path_escape(const std::string & str, bool escapeSlashes)
{
    const unsigned char *pnt, *path= (const unsigned char*)(str.c_str());
    char *p;
    size_t count = 0;

    for (pnt = (const unsigned char *)path; *pnt != '\0'; pnt++) {
        count += path_escape_ch(*pnt, escapeSlashes);
    }

    if (count == 0) {
        return std::string((const char*) path);
    }

    char buffer[str.size() + 2 * count + 1];
    p = buffer;
    for (pnt = (const unsigned char *)path; *pnt != '\0'; pnt++) {
    if (path_escape_ch(*pnt, escapeSlashes)) {
        /* Escape it - %<hex><hex> */
        sprintf(p, "%%%02X", (unsigned char) *pnt);
        p += 3;
    } else {
        *p++ = *pnt;
    }
    }
    *p = '\0';
    return std::string(buffer);
}

#undef path_escape_ch

std::ostream& operator<< (std::ostream& stream, const Davix::Uri & _u){
    stream << _u.getString();
    return stream;
}

} // namespace Davix


std::ostream& operator<< (std::ostream& stream, const Davix::Uri & _u){
	stream << _u.getString();
	return stream;
}
