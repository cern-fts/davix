#include "davix_listdir.hpp"
#include <global_def.hpp>
#include <core.hpp>
#include <davix_stat.hpp>
#include <xmlpp/webdavpropparser.hpp>

static const std::string simple_listing("<propfind xmlns=\"DAV:\"><prop></prop></propfind>");

static const std::string stat_listing("<propfind xmlns=\"DAV:\"><prop><getlastmodified/><creationdate/><getcontentlength/><resourcetype><collection/></resourcetype><mode/></prop></propfind>");

namespace Davix {

struct DIR_handle{
    DIR_handle(HttpRequest* _req, WebdavPropParser * _parser){
        request = _req;
        parser = _parser;
        size_t s_dirent = sizeof(struct dirent) + NAME_MAX +1;
        dir_info = (struct dirent*) calloc(1, s_dirent);
    }

   ~DIR_handle(){
    delete request;
    delete parser;
    free(dir_info);
    }

   HttpRequest* request;
   WebdavPropParser * parser;
   struct dirent* dir_info;

};

static void fill_dirent_from_filestat(struct dirent * d, const FileProperties & f, const off_t offset){
    d->d_off = offset;
    g_strlcpy(d->d_name, f.filename.c_str(), NAME_MAX);
}

int incremental_propfind_listdir_parsing(HttpRequest* req, WebdavPropParser * parser, size_t s_buff, const char* scope){
  //  std::cout << "time 1 pre-fecth" << time(NULL) << std::endl;
    char buffer[s_buff+1];
    const size_t ret_s_buff= req->read_block(buffer, s_buff);
    buffer[ret_s_buff]= '\0';
   davix_log_debug("chunk parse : result content : %s", buffer);
   (void) parser->parser_properties_from_chunk( Glib::ustring(buffer));
   //std::cout << "time 3 post-parse" << time(NULL) << std::endl;
   return ret_s_buff;
}




void configure_req_for_listdir(HttpRequest* req){
    req->add_header_field("Depth","1");
    req->set_requestcustom("PROPFIND");
}

DAVIX_DIR* Core::internal_opendirpp(const char * scope, const std::string & body, const std::string & url  ){
    size_t s_resu;
    int errno_err, error;
    DAVIX_DIR* r = NULL;
    try{


        // create a new connexion + parser for this opendir
        HttpRequest* http_req = static_cast<HttpRequest*>(_fsess->create_request( url));
        configure_req_for_listdir(http_req);
        std::auto_ptr<DIR_handle> res(  new DIR_handle(http_req, new WebdavPropParser()));
        time_t timestamp_timeout = time(NULL) + _timeout;

        HttpRequest *req = res->request;
        WebdavPropParser* parser = res->parser;
        // setup the handle for simple listing only
        req->add_full_request_content(body);

        req->execute_block(); // start req

        error= req->get_request_code(); // get errcode and test request validity
       if( (errno_err = httpcode_to_errno(error)) != 0){
           std::ostringstream os;
           os << " Error Webdav propfind : " << strerror(errno_err) << ", http errcode " << error << std::endl;
           throw Glib::Error(Glib::Quark(scope), errno_err, os.str());
       }

        size_t prop_size = 0;
        do{ // parse the begining of the request until the first property -> directory property
           s_resu = incremental_propfind_listdir_parsing(req, parser, this->_s_buff, scope);
           prop_size = parser->get_current_properties().size();

           if(s_resu < _s_buff && prop_size <1) // verify request status : if req done + no data -> error
               throw Glib::Error(Glib::Quark(scope), ECOMM, "Invalid Webdav result : invalid response content, maybe not a webdav server");

           if(timestamp_timeout < time(NULL))
         throw Glib::Error(Glib::Quark(scope), ECOMM, "Timeout on the request, Webdav content flow too slow");

        }while( prop_size < 1); // leave is end of req & no data

        r = res.release(); // success : take ownership of the pointer

    }catch(Glib::Error & e){
        throw e;
    }catch(xmlpp::exception & e){
        throw Glib::Error(Glib::Quark(scope), EINVAL, std::string("Parsing Error :").append(e.what()));
    }catch(std::exception & e){
        throw Glib::Error(Glib::Quark(scope), EINVAL, std::string("Unexcepted Error :").append(e.what()));
    }
    return (DAVIX_DIR*) r;
}


DAVIX_DIR* Core::opendir(const std::string &url){

    davix_log_debug(" -> davix_opendir");
    DAVIX_DIR* r = internal_opendirpp("Core::opendir",simple_listing, url);

    davix_log_debug(" <- davix_opendir");
    return (DAVIX_DIR*) r;
}

DAVIX_DIR* Core::opendirpp(const std::string &url){

    davix_log_debug(" -> davix_opendirpp");
    DAVIX_DIR* r = internal_opendirpp("Core::opendir",stat_listing, url);

    davix_log_debug(" <- davix_opendirpp");
    return (DAVIX_DIR*) r;
}


struct dirent* Core::readdir(DAVIX_DIR * d){
    davix_log_debug(" -> davix_readdir");
    if( d==NULL)
        throw Glib::Error(Glib::Quark("Core::readdir"), EBADF, "Invalid file descriptor for DAVIX_DIR*");
    DIR_handle* handle = static_cast<DIR_handle*>(d);

    try{
        HttpRequest *req = handle->request; // setup env again
        WebdavPropParser* parser = handle->parser;
        off_t read_offset = handle->dir_info->d_off+1;
        size_t prop_size = parser->get_current_properties().size();
        size_t s_resu = _s_buff;

        while(read_offset > ((off_t)prop_size)-1 && s_resu > 0){ // request not complete and current data too smalls
            // continue the parsing until one more result
           s_resu = incremental_propfind_listdir_parsing(req, parser, this->_s_buff, "Davix::readdir");
           prop_size = parser->get_current_properties().size();
        }
        if(read_offset > ((off_t)prop_size)-1) // end of the request, end of the story
            return NULL;
        fill_dirent_from_filestat(handle->dir_info, parser->get_current_properties().at(read_offset), read_offset);
        davix_log_debug(" <- davix_readdir");
        return handle->dir_info;

    }catch(Glib::Error & e){
        throw e;
    }catch(xmlpp::exception & e){
        throw Glib::Error(Glib::Quark("Davix::readdir"), EINVAL, std::string("Parsing Error :").append(e.what()));
    }catch(std::exception & e){
        throw Glib::Error(Glib::Quark("Davix::readdir"), EINVAL, std::string("Unexcepted Error :").append(e.what()));
    }
    return NULL;
}

struct dirent* Core::readdirpp(DAVIX_DIR * d, struct stat *st){
    davix_log_debug(" -> davix_readdirpp");
    if( d==NULL)
        throw Glib::Error(Glib::Quark("Core::readdirpp"), EBADF, "Invalid file descriptor for DAVIX_DIR*");
    DIR_handle* handle = static_cast<DIR_handle*>(d);

    try{
        HttpRequest *req = handle->request; // setup env again
        WebdavPropParser* parser = handle->parser;
        off_t read_offset = handle->dir_info->d_off+1;
        size_t prop_size = parser->get_current_properties().size();
        size_t s_resu = _s_buff;

        while(read_offset > ((off_t)prop_size)-1 && s_resu > 0){ // request not complete and current data too smalls
            // continue the parsing until one more result
           s_resu = incremental_propfind_listdir_parsing(req, parser, this->_s_buff, "Davix::readdirpp");
           prop_size = parser->get_current_properties().size();
        }
        if(read_offset > ((off_t)prop_size)-1) // end of the request, end of the story
            return NULL;
        fill_dirent_from_filestat(handle->dir_info, parser->get_current_properties().at(read_offset), read_offset);
        fill_stat_from_fileproperties(st, parser->get_current_properties().at(read_offset));
        davix_log_debug(" <- davix_readdirpp");
        return handle->dir_info;

    }catch(Glib::Error & e){
        throw e;
    }catch(xmlpp::exception & e){
        throw Glib::Error(Glib::Quark("Davix::readdirpp"), EINVAL, std::string("Parsing Error :").append(e.what()));
    }catch(std::exception & e){
        throw Glib::Error(Glib::Quark("Davix::readdirpp"), EINVAL, std::string("Unexcepted Error :").append(e.what()));
    }
    return NULL;
}


void Core::closedirpp(DAVIX_DIR * d){
    if( d==NULL)
        throw Glib::Error(Glib::Quark("Davix::Closedir"), EBADF, "Invalid file descriptor for DAVIX_DIR*");
    delete (static_cast<DIR_handle*>(d));
}

void Core::closedir(DAVIX_DIR * d){
    return closedirpp(d);
}

} // namespace Davix
