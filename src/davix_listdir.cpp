#include "davix_listdir.h"
#include <global_def.h>
#include <xmlpp/webdavpropparser.h>



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
   std::vector<char>  buffer;
   buffer.reserve(s_buff+1);
   req->read_block(buffer, s_buff);
   const size_t ret_s_buff = buffer.size();
   buffer.push_back('\0');
   //std::cout << "content "<< &(buffer.at(0)) << std::endl;
   //std::cout << "time 2 pre-parse" << time(NULL) << std::endl;
   davix_log_debug("chunk parse : result content : %s",&(buffer[0]));
   (void) parser->parser_properties_from_chunk( Glib::ustring(&(buffer.front())));
   //std::cout << "time 3 post-parse" << time(NULL) << std::endl;
   return ret_s_buff;
}




void configure_req_for_listdir(HttpRequest* req){
    req->add_header_field("Depth","1");
    req->set_requestcustom("PROPFIND");
}


DAVIX_DIR* Core::opendir(const std::string &url){
    size_t s_resu;
    int errno_err, error;
    davix_log_debug(" -> davix_opendir");
    DAVIX_DIR* r = NULL;
    try{
        // create a new connexion + parser for this opendir
        HttpRequest* http_req = static_cast<HttpRequest*>(_fsess->take_request(HTTP, url));
        configure_req_for_listdir(http_req);
        std::auto_ptr<DIR_handle> res(  new DIR_handle(http_req, new WebdavPropParser()));
        time_t timestamp_timeout = time(NULL) + _timeout;

        HttpRequest *req = res->request;
        WebdavPropParser* parser = res->parser;

        req->execute_block(_s_buff); // start req

        error= req->get_request_code(); // get errcode and test request validity
       if( (errno_err = httpcode_to_errno(error)) != 0){
           std::ostringstream os;
           os << " Error Webdav propfind : " << strerror(errno_err) << ", http errcode " << error << std::endl;
           throw Glib::Error(Glib::Quark("Core::opendir"), errno_err, os.str());
       }

        size_t prop_size = 0;
        do{ // parse the begining of the request until the first property -> directory property
           s_resu = incremental_propfind_listdir_parsing(req, parser, this->_s_buff, "Davix::opendir");
           prop_size = parser->get_current_properties().size();

           if(s_resu < _s_buff && prop_size <1) // verify request status : if req done + no data -> error
               throw Glib::Error(Glib::Quark("Davix::Opendir"), ECOMM, "Invalid Webdav result : invalid response content, maybe not a webdav server");

           if(timestamp_timeout < time(NULL))
         throw Glib::Error(Glib::Quark("Davix::Opendir"), ECOMM, "Timeout on the request, Webdav content flow too slow");

        }while( prop_size < 1); // leave is end of req & no data

        r = res.release(); // success : take ownership of the pointer
        davix_log_debug(" <- davix_opendir");
    }catch(Glib::Error & e){
        throw e;
    }catch(xmlpp::exception & e){
        throw Glib::Error(Glib::Quark("Davix::Opendir"), EINVAL, std::string("Parsing Error :").append(e.what()));
    }catch(std::exception & e){
        throw Glib::Error(Glib::Quark("Davix::Opendir"), EINVAL, std::string("Unexcepted Error :").append(e.what()));
    }
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

        while(read_offset > ((off_t)prop_size)-1 && s_resu == _s_buff){ // request not complete and current data too smalls
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
        throw Glib::Error(Glib::Quark("Davix::Opendir"), EINVAL, std::string("Parsing Error :").append(e.what()));
    }catch(std::exception & e){
        throw Glib::Error(Glib::Quark("Davix::Opendir"), EINVAL, std::string("Unexcepted Error :").append(e.what()));
    }
    return NULL;
}


void Core::closedir(DAVIX_DIR * d){
    if( d==NULL)
        throw Glib::Error(Glib::Quark("Davix::Closedir"), EBADF, "Invalid file descriptor for DAVIX_DIR*");
    delete (static_cast<DIR_handle*>(d));
}

} // namespace Davix
