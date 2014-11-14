#ifndef DAVIX_DEPRECATED
#define DAVIX_DEPRECATED

#include <vector>
#include <string>
#include <utils/davix_types.hpp>
#include <utils/davix_uri.hpp>

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif

/**
  @file deprecated.hpp
  @author Devresse Adrien

  @brief Deprecated class / functions of Davix for ABI/ API compatibility

   Each call to one of these function kill a kitten.
   Please love kittens and don't use them
 */

#ifndef DOXYGEN_SHOULD_SKIP_THIS



namespace Davix {

struct HttpCacheTokenInternal;


class DAVIX_EXPORT HttpCacheToken
{
public:

    HttpCacheToken();

    HttpCacheToken(const HttpCacheToken & orig);

    virtual ~HttpCacheToken();

    HttpCacheToken & operator=(const HttpCacheToken &);

    const Uri & getrequestUri() const;


    const Uri & getCachedRedirection() const;
private:
    HttpCacheTokenInternal* d_ptr;

    friend struct HttpCacheTokenAccessor;
};


class DAVIX_EXPORT FileInfoInterface{
public:
    virtual ~FileInfoInterface(){}

    virtual const std::type_info & getType() =0;

    virtual FileInfoInterface* getClone() = 0;
};


template <class T>
class DAVIX_EXPORT FileInfo : public FileInfoInterface{
public:
    virtual ~FileInfo(){}

    virtual const std::type_info & getType(){
        return typeid(*static_cast<T*>(this));
    }

    virtual FileInfoInterface* getClone(){
        return new T(*static_cast<T*>(this));
    }
};



class DAVIX_EXPORT FileInfoSize :
        public FileInfo<FileInfoSize>{
public:
    FileInfoSize() : size(0){}
    FileInfoSize(dav_size_t s) : size(s){}
    FileInfoSize(const FileInfoSize & orig) : size(orig.size) {}
    virtual ~FileInfoSize(){}
    dav_size_t size;
};


class DAVIX_EXPORT FileInfoHash :
    public FileInfo<FileInfoHash>{
public:
    FileInfoHash(const FileInfoHash & orig) : hash(orig.hash), hash_type(orig.hash_type) {}
    virtual ~FileInfoHash(){}
    std::string hash;
    std::string hash_type;
};


class DAVIX_EXPORT FileInfoName :
    public FileInfo<FileInfoName>{
public:
    FileInfoName(const FileInfoName & orig) : name(orig.name){}
    virtual ~FileInfoName(){}
    std::string name;
};



class DAVIX_EXPORT FileInfoProtocolType :
    public FileInfo<FileInfoProtocolType>{
public:
    FileInfoProtocolType() : protocol(){}
    FileInfoProtocolType(const FileInfoProtocolType & orig) : protocol(orig.protocol){}
    FileInfoProtocolType(const std::string & s) : protocol(s){}
    virtual ~FileInfoProtocolType(){}
    std::string protocol;
};




typedef std::vector<FileInfoInterface*> Properties;


class DAVIX_EXPORT Replica {
public:
    Replica() :
        uri(),
        props(){}
    Replica(const Replica & orig) :
        uri(orig.uri),
        props() {
        props.reserve(orig.props.size());
        for(Properties::iterator it = props.begin(); it < props.end(); ++it){
            props.push_back((*it)->getClone());
        }
    }

    virtual ~Replica(){
        for(Properties::iterator it = props.begin(); it < props.end(); ++it)
            delete *it;
    }

    Uri uri;
    Properties props;
};

typedef std::deque<Replica> ReplicaVec;



struct HookIntern;

///
/// \brief The HookTraits class
///
///  Base class for Daivx Hook functions
///
struct HookTraits{
    HookTraits();
    virtual ~HookTraits();

    virtual const std::type_info & getType() =0;

    // internal
    HookIntern* d_ptr;
};


template <class T>
struct Hook: HookTraits{
    virtual const std::type_info & getType(){
        return typeid(*static_cast<T>(this));
    }
};

class HttpRequest;

typedef void (*CallbackHeader)(HttpRequest &, const std::string &, std::string &);


struct HookSendHeader : public Hook<HookSendHeader>{
    CallbackHeader hook;
};


struct HookReceiveHeader : public Hook<HookReceiveHeader>{
    CallbackHeader hook;
};


typedef void (*CallbackRequestExec)(HttpRequest &);



struct HookRequestPreExec : public Hook<HookRequestPreExec>{
    CallbackRequestExec hook;
};


struct HookRequestPostExec : public Hook<HookRequestPostExec>{
    CallbackRequestExec hook;
};

} // namespace Davix


#endif

#endif
