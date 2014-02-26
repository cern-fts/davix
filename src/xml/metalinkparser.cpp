#include <davix_internal.hpp>

#include "metalinkparser.hpp"
#include <logger/davix_logger_internal.h>
#include <string_utils/stringutils.hpp>

namespace Davix{

const std::string MetalinkScope = "MetalinkParser";

const std::string tags_string[] = { "metalink", "files", "file",  "size", "resources", "url" };
const size_t tags_string_size = (sizeof(tags_string))/((sizeof(std::string )));

const MetalinkTag::MetalinkParserTag url_stack[] = { MetalinkTag::Metalink,
                                            MetalinkTag::Files, MetalinkTag::File, MetalinkTag::Resources, MetalinkTag::Url };
const size_t url_stack_size= (sizeof(url_stack))/(sizeof(MetalinkTag::MetalinkParserTag));

const MetalinkTag::MetalinkParserTag url_stack_meta4[] = { MetalinkTag::Metalink, MetalinkTag::File, MetalinkTag::Url };
const size_t url_stack_size_meta4= (sizeof(url_stack_meta4))/(sizeof(MetalinkTag::MetalinkParserTag));

const MetalinkTag::MetalinkParserTag size_stack[] = { MetalinkTag::Metalink,
                                            MetalinkTag::Files, MetalinkTag::File, MetalinkTag::Size };
const size_t size_stack_size= (sizeof(size_stack))/(sizeof(MetalinkTag::MetalinkParserTag));

const MetalinkTag::MetalinkParserTag size_stack_meta4[] = { MetalinkTag::Metalink, MetalinkTag::File, MetalinkTag::Size };
const size_t size_stack_size_meta4= (sizeof(size_stack_meta4))/(sizeof(MetalinkTag::MetalinkParserTag));

static MetalinkTag::MetalinkParserTag getTag(const std::string & str){
    const std::string* p =  std::find(tags_string, tags_string + tags_string_size, str);
    if(p < ( tags_string + tags_string_size))
            return static_cast<MetalinkTag::MetalinkParserTag>(p-tags_string);
    return MetalinkTag::Invalid;
}


static bool matchStack(const MetalinkStack & s1, const MetalinkTag::MetalinkParserTag* tab, size_t s_tab){
    if(s1.size() != s_tab)
        return false;
    return std::equal(s1.begin(), s1.end(), tab);
}



struct MetalinkParser::MetalinkParserIntern{
    MetalinkParserIntern(Context & c, std::vector<DavFile> & fvec) : _c(c), _fvec(fvec), _tagStack(), _filesize(0){
        _tagStack.reserve(5);
    }

    inline int startElem(const std::string & name){

        DAVIX_TRACE("MetalinkParser: <tag> %s", name.c_str());
        const MetalinkTag::MetalinkParserTag t = getTag(name);
        if( t == MetalinkTag::Invalid){
            return 0;
        }
        _tagStack.push_back(t);
        return 1;
    }

    inline int dataElem(const char* data, size_t len){
        _buffer.reserve(_buffer.size()+len+1);
        std::copy(data, data+len, std::back_inserter(_buffer));
        return 0;
    }


    inline int endElem(const std::string & name){
        const MetalinkTag::MetalinkParserTag t = getTag(name);
        std::string & replic = _buffer;

        // metalink 3.0
        if(matchStack(_tagStack, url_stack, url_stack_size)){
            DAVIX_TRACE("MetalinkParser 3.0 : Replica URL %s", replic.c_str());
            _fvec.push_back(File(_c, Uri(trim<int (*)(int)>(replic, std::isspace))));
        }
        if(matchStack(_tagStack, size_stack, size_stack_size)){
            DAVIX_TRACE("MetalinkParser 3.0 : Replica size %d", replic.c_str());
            _filesize = static_cast<dav_size_t>(strtoul(replic.c_str(), NULL, 10));
        }

        // metalink 4.0
        if(matchStack(_tagStack, url_stack_meta4, url_stack_size_meta4)){
            DAVIX_TRACE("MetalinkParser 4.0 : Replica URL %s", replic.c_str());
            _fvec.push_back(File(_c, Uri(trim<int (*)(int)>(replic, std::isspace))));
        }
        if(matchStack(_tagStack, size_stack_meta4, size_stack_size_meta4)){
            DAVIX_TRACE("MetalinkParser 4.0 : Replica size %d", replic.c_str());
            _filesize = static_cast<dav_size_t>(strtoul(replic.c_str(), NULL, 10));
        }

        _buffer.clear();

        if(t == _tagStack.back())
            _tagStack.pop_back();
        return 0;
    }

    Context & _c;
    std::vector<DavFile> & _fvec;
    MetalinkStack _tagStack;
    dav_size_t _filesize;
    std::string _buffer;
};


MetalinkParser::MetalinkParser(Context & u, std::vector<DavFile> & vec) : d_ptr(new MetalinkParserIntern(u, vec))
{

}



MetalinkParser::~MetalinkParser(){
    delete d_ptr;
}


dav_size_t MetalinkParser::getSize() const{
    return d_ptr->_filesize;
}


int MetalinkParser::parserStartElemCb(int parent,
                               const char *nspace, const char *name,
                               const char **atts){
    return d_ptr->startElem(name);
}

int MetalinkParser::parserCdataCb(int state,
                            const char *cdata, size_t len){

    return d_ptr->dataElem(cdata, len);
}

int MetalinkParser::parserEndElemCb(int state,
                            const char *nspace, const char *name){

    return d_ptr->endElem(name);
}


}
