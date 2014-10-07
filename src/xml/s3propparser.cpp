#include "s3propparser.hpp"

#include <stack>
#include <utils/davix_logger_internal.hpp>


namespace Davix{

const std::string delimiter_prop ="Contents";
const std::string name_prop = "Key";
const std::string size_prop = "Size";

struct S3PropParser::Internal{
    std::string current;
    std::stack<std::string> stack_status;
    std::deque<FileProperties> props;

    FileProperties property;

    int start_elem(const std::string &elem){
        // new tag, clean content;
        current.clear();

        // check XML nested level, security protection
        if(stack_status.size() < 200){
            stack_status.push(elem);
        }else{
            throw DavixException(davix_scope_xml_parser(), StatusCode::ParsingError, "Impossible to parse S3 content, corrupted XML");
        }

        // check element, if new entry clear current entry
        if( StrUtil::compare_ncase(delimiter_prop, elem) ==0){
            DAVIX_TRACE("new element found", elem.c_str());
            property.clear();
        }

        return 1;
    }

    int add_chunk(const std::string & chunk){
        current.append(chunk);
        return 0;
    }

    int end_elem(const std::string &elem){

        // new name new fileprop
        if(StrUtil::compare_ncase(name_prop, elem) ==0){
            DAVIX_TRACE("new element %s", elem.c_str());
            property.filename = current;
        }

        if(StrUtil::compare_ncase(size_prop, elem) ==0){
            try{
                unsigned long size = toType<unsigned long, std::string>()(elem);
                DAVIX_TRACE("element size %ld", size);
            }catch(...){
                DAVIX_TRACE("Unable to parse element size");
            }
        }

        // check element, if end entry push new entry
        if( StrUtil::compare_ncase(delimiter_prop, elem) ==0){
            DAVIX_TRACE("push new element %s", elem.c_str());
            props.push_back(property);
        }


        // reduce stack size
        if(stack_status.size() > 0)
            stack_status.pop();
        current.clear();
        return 0;
    }

};


S3PropParser::S3PropParser() : d_ptr(new Internal())
{
}

S3PropParser::~S3PropParser(){

}


int S3PropParser::parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts){
    (void) parent;
    (void) nspace;
    (void) atts;
    return d_ptr->start_elem(std::string(name));

}

int S3PropParser::parserCdataCb(int state, const char *cdata, size_t len){
    (void) state;
    (void) len;
    return d_ptr->add_chunk(std::string(cdata));
}

int S3PropParser::parserEndElemCb(int state, const char *nspace, const char *name){
    (void) state;
    (void) nspace;
    return d_ptr->end_elem(std::string(name));
}

std::deque<FileProperties> & S3PropParser::getProperties(){
    return d_ptr->props;
}


}
