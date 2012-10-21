#include "davix_stat.hpp"

#include <ostream>
#include <sstream>
#include <string>
#include <cstring>
#include <xmlpp/webdavpropparser.hpp>

#include <contextinternal.h>

namespace Davix{

void fill_stat_from_fileproperties(struct stat* st, const  FileProperties & prop){
    memset(st, 0, sizeof(struct stat));
    st->st_mtime = prop.mtime;
    st->st_atime = prop.atime;
    st->st_ctime = prop.ctime;
    st->st_size = prop.size;
    st->st_mode = prop.mode;
}


int DavPosix::stat(const RequestParams * _params, const std::string & url, struct stat* st, DavixError** err){
    davix_log_debug(" -> davix_stat");
    RequestParams params(_params);
    DavixError* tmp_err=NULL;

    int ret =-1;
    WebdavPropParser parser;
    std::auto_ptr<HttpRequest> req( static_cast<HttpRequest*>(context->_intern->getSessionFactory()->create_request(url, &tmp_err)));
    if( req.get() != NULL){
        req->set_parameters(params);

        const std::vector<char> & res = req_webdav_propfind(req.get(), &tmp_err);
        if(!tmp_err){
            const std::vector<FileProperties> & props = parser.parser_properties_from_memory(std::string(((char*) & res.at(0)), res.size()));

            if( props.size() < 1){
                DavixError::setupError(&tmp_err, davix_scope_stat_str(), Davix::StatusCode::WebDavPropertiesParsingError, "Parsing Error : properties number < 1");
            }else{
                fill_stat_from_fileproperties(st, props.front());
                ret =0;
        }
        }
        davix_log_debug(" davix_stat <-");
    }

    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;

}

/**
  execute a propfind/stat request on a given HTTP request handle
  return a vector with the content of the request if success
*/
const std::vector<char> & req_webdav_propfind(HttpRequest* req, DavixError** err){
    DavixError* tmp_err=NULL;

    req->addHeaderField("Depth","0");
    req->setRequestMethod("PROPFIND");
    int ret = req->execute_sync(&tmp_err);
    if(ret != 0)
        DavixError::propagateError(err, tmp_err);
    return req->get_result();
}


}


DAVIX_C_DECL_BEGIN

int davix_posix_stat(davix_sess_t sess, davix_params_t _params, const char* url, struct stat * st, davix_error_t* err){
    g_return_val_if_fail(sess != NULL,-1);

    Davix::DavPosix p((Davix::Context*)(sess));
    Davix::RequestParams * params = (Davix::RequestParams*) (_params);

    return p.stat(params,url, st, (Davix::DavixError**) err);

}

DAVIX_C_DECL_END
