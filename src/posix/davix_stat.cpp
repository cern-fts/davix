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


void DavPosix::stat(const RequestParams * _params, const std::string & url, struct stat* st){
    davix_log_debug(" -> davix_stat");
    RequestParams params(_params);

    try{
        WebdavPropParser parser;
        std::auto_ptr<HttpRequest> req( static_cast<HttpRequest*>(context->_intern->getSessionFactory()->create_request(url)));
        req->set_parameters(params);

        const std::vector<char> & res = req_webdav_propfind(req.get());
        const std::vector<FileProperties> & props = parser.parser_properties_from_memory(std::string(((char*) & res.at(0)), res.size()));

        if( props.size() < 1)
            throw Glib::Error(Glib::Quark("Davix::Stat::stat"), EINVAL, " Invalid Webdav response" );
        fill_stat_from_fileproperties(st, props.front());

        davix_log_debug(" davix_stat <-");
    }catch(Glib::Error & e){
        throw e;
    }catch(xmlpp::exception & e){
        throw Glib::Error(Glib::Quark("Davix::Stat"), EINVAL, std::string("Parsing Error :").append(e.what()));
    }catch(std::exception & e){
        throw Glib::Error(Glib::Quark("Davix::Stat"), EINVAL, std::string("Unexcepted Error :").append(e.what()));
    }

}

/**
  execute a propfind/stat request on a given HTTP request handle
  return a vector with the content of the request if success
  throw Glib::Error if error occures ( 404, etc.. )
*/
const std::vector<char> & req_webdav_propfind(HttpRequest* req){
    int errno_err, error = 404;
    req->addHeaderField("Depth","0");
    req->setRequestMethod("PROPFIND");
    (void) req->execute_sync();
     error = req->getRequestCode();
    if( (errno_err = httpcode_to_errno(error)) != 0){
        std::ostringstream os;
        os << " Error Webdav propfind : " << strerror(errno_err) << ", http errcode " << error << std::endl;
        throw Glib::Error(Glib::Quark("Davix::Stat"), errno_err, os.str());
    }
    return req->get_result();
}


}


DAVIX_C_DECL_BEGIN

int davix_posix_stat(davix_sess_t sess, davix_params_t _params, const char* url, struct stat * st, GError** err){
    g_return_val_if_fail(sess != NULL,-1);

    try{
        Davix::DavPosix p(static_cast<Davix::Context*>(sess));
        Davix::RequestParams * params = (Davix::RequestParams*) (_params);

        p.stat(params,url, st);
        return 0;
    }catch(Glib::Error & e){
        if(err)
            *err= g_error_copy(e.gobj());
    }catch(std::exception & e){
        g_set_error(err, g_quark_from_string("davix_stat"), EINVAL, "unexcepted error %s", e.what());
    }
    return -1;
}

DAVIX_C_DECL_END
