

#include <core.hpp>
#include <abstractsessionfactory.hpp>
#include <neon/neonsessionfactory.hpp>
#include <cstring>
#include "test_request.h"

using namespace Davix;

void test_request_chunk(){
    // prepare local files
    char* tmp_file = tempnam(NULL, "test_file_request_");
    const char * content = "Hello";
    const size_t  times = 4000;

    FILE* f = fopen(tmp_file, "w+");
    if(f == NULL){
        assert_true_with_message(FALSE, " unable to create tmp_file");
        return;
    }
    for(size_t i=0; i < times; ++i){
        fwrite(content, strlen(content), 1, f);
    }
    fseek(f, 0,SEEK_SET);

    try{
        char buffer[6000]; // original data
        const size_t s_read=20;

        std::auto_ptr<AbstractSessionFactory> s( new NEONSessionFactory());
        std::string tmp_file_url("file:///"); tmp_file_url.append(tmp_file);
        std::cout << " file url :" << tmp_file_url << std::endl;
        Request* r(s->create_request(tmp_file_url));
        char v[6000];
        r->execute_block();
        ssize_t n = r->read_block(v,s_read);
        v[n] = '\0';
        fread(buffer,1,s_read, f);
        buffer[s_read] = '\0';
        assert_true_with_message(strncmp(buffer, v, s_read) ==0 , "must be the same content %s | %s", buffer,v);


        const size_t s_read2 = 60;
        memset(v, 0, 6000);
        n= r->read_block(v, s_read2);
        v[n] = '\0';
        fread(buffer,1,s_read2, f);
        buffer[s_read2] = '\0';
        assert_true_with_message(strncmp(buffer, v, s_read2) ==0 , "must be the same content %s %s 2 ", buffer, v);
        s->delete_request(r);
    }catch(Glib::Error & e){
        assert_true_with_message(FALSE, " error occures : N° %d %s", e.code(), e.what().c_str());
    }catch(std::exception & e){
        assert_true_with_message(FALSE, " unknow error occures :  %s", e.what());
    }
    free(tmp_file);
    fclose(f);
}


TestSuite * request_suite (void)
{
        TestSuite *s2 = create_test_suite();
        // verbose test case /
        add_test(s2, test_request_chunk);

        return s2;
 }
