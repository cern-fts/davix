#include "davix_mkdir.hpp"

#include <ostream>
#include <sstream>

#include <global_def.hpp>
#include <davixcontext.hpp>
#include <xmlpp/webdavpropparser.hpp>


namespace Davix{

void DavPosix::mkdir(const RequestParams * _params, const std::string &url, mode_t right){
    davix_log_debug(" -> davix_mkdir");
    int error, errno_err;
    RequestParams params(_params);

    try{
        WebdavPropParser parser;
        std::auto_ptr<HttpRequest> req( static_cast<HttpRequest*>(context->_intern->getSessionFactory()->create_request(url)));
        req->set_parameters(params);

        req->setRequestMethod("MKCOL");

        req->execute_sync();

        error= req->getRequestCode(); // get errcode and test request validity
        if( (errno_err = httpcode_to_errno(error)) != 0){
           std::ostringstream os;
           os << " Error Webdav propfind : " << strerror(errno_err) << ", http errcode " << error << std::endl;
           throw Glib::Error(Glib::Quark("Core::mkdir"), errno_err, os.str());
        }


    }catch(Glib::Error & e){
        throw e;
    }catch(xmlpp::exception & e){
        throw Glib::Error(Glib::Quark("Davix::mkdir"), EINVAL, std::string("Parsing Error :").append(e.what()));
    }catch(std::exception & e){
        throw Glib::Error(Glib::Quark("Davix::mkdir"), EINVAL, std::string("Unexcepted Error :").append(e.what()));
    }
    davix_log_debug(" davix_mkdir <-");
}




DAVIX_C_DECL_BEGIN

int davix_posix_mkdir(davix_sess_t sess, davix_params_t _params, const char* url,  mode_t right, GError** err){
    g_return_val_if_fail(sess != NULL && url != NULL,-1);

    try{
        Davix::DavPosix p(static_cast<Davix::Context*>(sess));
        Davix::RequestParams * params = (Davix::RequestParams*) (_params);

        p.mkdir(params,url, right);
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


}
