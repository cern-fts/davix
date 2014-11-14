
#include <davix.hpp>
#include <cstring>
#include <memory>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <exception>
#include <cassert>

#include "davix_test_lib.h"

using namespace Davix;



int main(int argc, char** argv){
    if( argc < 2){
        std::cout << "Usage  : " << std::endl;
        std::cout <<"\t" << argv[0] << " [url]" << std::endl;
        std::cout <<"\t" << argv[0] << " [url] [CERTIFICATE_PATH] " << std::endl;
        return 0;
    }

    srand(time(NULL));

    davix_set_log_scope(SCOPE_ALL);
    davix_set_log_level(LOG_ALL);

    RequestParams p;
    Context context;



    std::string url = argv[1];
    Uri u =  generate_random_uri(Davix::Uri(url), "test_davfile_read");

    if(argc > 2){
        configure_grid_env(argv[2], p);
    }


   std::string origin = "Obiwan Kenobi, Yoda Master, Skywalker, Wookie, Light saber";
   std::string str = origin;
   std::istringstream istr(str);


    // Try with streams
   try{


       std::cout << " set File " << std::endl;
       File f(context, p, u);
       istr >> std::noskipws;
       istr >> f;

       StatInfo info;
       File other(context, u);
       other.statInfo(&p, info);

       std::cout << "infos" << info.size << " " << origin.size() << std::endl;
       assert(info.size == origin.size());

       std::ostringstream ss;
       ss << other;
       std::string result(ss.str());

       assert( result.compare(origin) ==0);

       other.deletion(&p);

   }catch(std::exception & e){
        std::cerr << "Execution error" << e.what() << std::endl;
        return -1;
   }

 /*   try{
       File f(context, p, u);

       StatInfo info;
       f.statInfo(&p, info);
       std::cout << " arrive to execute stat after deletion: error " << u << " " << info.size << std::endl;
       assert(false);
   }catch(DavixException & e){
        assert(e.code() == StatusCode::FileNotFound);
   }*/



   /// normal content, non text + put / get schema
   ///
   Uri u2 =  generate_random_uri(Davix::Uri(url), "test_davfile_read_binary");

   dav_size_t size_data = rand()%(1 <<20) +2;

   char data_buffer[size_data];

   for(dav_size_t i =0; i < size_data; ++i){
       data_buffer[i] = static_cast<char>(rand());
   }


   File fbin(context, p, u2);
   fbin.put(NULL, data_buffer, size_data);

   std::vector<char> rece_buffer;
   File fbin_rece(context, p, u2);
   fbin_rece.get(NULL, rece_buffer);

   assert(memcmp(data_buffer, &(rece_buffer.at(0)), size_data) == 0);

   fbin_rece.deletion();

    return 0;
}

