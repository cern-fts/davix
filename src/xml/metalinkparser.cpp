#include "metalinkparser.hpp"
#include <cstring>
#include <algorithm>
#include <functional>
#include <logger/davix_logger_internal.h>

namespace Davix{

const std::string MetalinkScope = "MetalinkParser";


const std::string tags_string[] = { "metalink", "files", "file",  "size", "resources", "url" };
const size_t tags_string_size = 6;

const MetalinkTag::MetalinkParserTag url_stack[] = { MetalinkTag::Metalink3,
                                            MetalinkTag::Files, MetalinkTag::File, MetalinkTag::Resources, MetalinkTag::Url };
const size_t url_stack_size= 5;

const MetalinkTag::MetalinkParserTag size_stack[] = { MetalinkTag::Metalink3,
                                            MetalinkTag::Files, MetalinkTag::File, MetalinkTag::Size };
const size_t size_stack_size= 4;
/*
static bool isMatching(MetalinkTag::MetalinkParserTag tag, const char* name){
    return (tags_string[static_cast<int>(tag)].compare( name) ==0);
}*/

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



MetalinkParser::MetalinkParser() :
    rep(new ReplicaVec()),
    fileProperties(new Properties()),
    tagStack()
{
    //rep.reserve(5);
    tagStack.reserve(5);
}

MetalinkParser::MetalinkParser(ReplicaVec &reps, Properties &props) :
    rep(&reps),
    fileProperties(&props)
{
     tagStack.reserve(5);
}

MetalinkParser::~MetalinkParser(){
    delete rep;
    delete fileProperties;
}

const ReplicaVec & MetalinkParser::getReplicas(){
    return *rep;
}

const Properties & MetalinkParser::getProps(){
    return *fileProperties;
}

int MetalinkParser::parserStartElemCb(int parent,
                               const char *nspace, const char *name,
                               const char **atts){
    DAVIX_TRACE("MetalinkParser: <tag> %s", name);
    const MetalinkTag::MetalinkParserTag t = getTag(std::string(name));
    if( t == MetalinkTag::Invalid){
        return 0;
    }
    tagStack.push_back(t);
    if(t == MetalinkTag::Url){
        // create new replicas
        rep->resize(rep->size()+1);
        // get replicas type
        for(const char** p = atts; *p != NULL; p+=2){
            if( *(p+1) == NULL)
                break;
            if( strcmp(*p,"type") == 0){
                DAVIX_TRACE("MetalinkParser: URL type %s=%s", *p, *(p+1));
                rep->back().props.push_back(new FileInfoProtocolType(*(p+1)));
                break;
            }
        }
    }
    return 1;
}

int MetalinkParser::parserCdataCb(int state,
                            const char *cdata, size_t len){

    if(matchStack(tagStack, url_stack, url_stack_size)){
        std::string replic(cdata, len);
        DAVIX_TRACE("MetalinkParser: Replica URL %s", replic.c_str());
        rep->back().uri = replic;
    }
    if(matchStack(tagStack, size_stack, size_stack_size)){
        const std::string replic_size(cdata, len);
        DAVIX_TRACE("MetalinkParser: Replica size %d", replic_size.c_str());
        fileProperties->push_back( new FileInfoSize(strtoul(replic_size.c_str(), NULL, 10)));
    }
    return 0;
}

int MetalinkParser::parserEndElemCb(int state,
                            const char *nspace, const char *name){

    const MetalinkTag::MetalinkParserTag t = getTag(std::string(name));
    if(t == tagStack.back())
        tagStack.pop_back();
    return 0;
}


}
