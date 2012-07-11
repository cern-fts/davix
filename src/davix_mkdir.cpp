#include "davix_mkdir.hpp"
#include <global_def.hpp>
#include <core.hpp>
#include <xmlpp/webdavpropparser.hpp>


namespace Davix{

void Core::mkdir(const std::string &url, mode_t right){
    davix_log_debug(" -> davix_mkdir");
    int error, errno_err;

    try{
        WebdavPropParser parser;
        std::auto_ptr<HttpRequest> req( static_cast<HttpRequest*>(_fsess->take_request(HTTP, url)));

        req->set_requestcustom("MKCOL");

        req->execute_sync();

        error= req->get_request_code(); // get errcode and test request validity
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

}
