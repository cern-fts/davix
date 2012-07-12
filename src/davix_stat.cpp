#include "davix_stat.hpp"
#include <string>
#include <cstring>
#include <xmlpp/webdavpropparser.hpp>


void Davix::fill_stat_from_fileproperties(struct stat* st, const  FileProperties & prop){
    memset(st, 0, sizeof(struct stat));
    st->st_mtime = prop.mtime;
    st->st_atime = prop.atime;
    st->st_ctime = prop.ctime;
    st->st_size = prop.size;
    st->st_mode = prop.mode;
}


void Davix::Core::stat(const std::string & url, struct stat* st){
    davix_log_debug(" -> davix_stat");

    try{
        WebdavPropParser parser;
        std::auto_ptr<HttpRequest> req( static_cast<HttpRequest*>(_fsess->create_request(url)));

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
const std::vector<char> & Davix::req_webdav_propfind(HttpRequest* req){
    int errno_err, error = 404;
    req->add_header_field("Depth","0");
    req->set_requestcustom("PROPFIND");
    (void) req->execute_sync();
     error = req->get_request_code();
    if( (errno_err = httpcode_to_errno(error)) != 0){
        std::ostringstream os;
        os << " Error Webdav propfind : " << strerror(errno_err) << ", http errcode " << error << std::endl;
        throw Glib::Error(Glib::Quark("Davix::Stat"), errno_err, os.str());
    }
    return req->get_result();
}
