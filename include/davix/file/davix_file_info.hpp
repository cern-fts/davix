#ifndef DAVIX_FILE_INFO_HPP
#define DAVIX_FILE_INFO_HPP


#include <davix_file_types.hpp>
#include <davixuri.hpp>

/**
  @file davix_file_info.hpp
  @author Devresse Adrien

  @brief davix file Meta-data class

  @warning Unstable API for preview only now, subject to changes.
*/

namespace Davix{

///
/// \brief Generic container Traits
///
///  Base struct for any generic
///
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


///
/// @class FileInfo
/// \brief File size property
///
class DAVIX_EXPORT FileInfoSize :
        public FileInfo<FileInfoSize>{
public:
    FileInfoSize() : size(0){}
    FileInfoSize(dav_size_t s) : size(s){}
    FileInfoSize(const FileInfoSize & orig) : size(orig.size) {}
    virtual ~FileInfoSize(){}
    dav_size_t size;
};

///
/// @class FileInfo
/// \brief File Hash property
///
class DAVIX_EXPORT FileInfoHash :
    public FileInfo<FileInfoHash>{
public:
    FileInfoHash(const FileInfoHash & orig) : hash(orig.hash), hash_type(orig.hash_type) {}
    virtual ~FileInfoHash(){}
    std::string hash;
    std::string hash_type;
};

///
/// @class FileInfo
/// \brief File alternative name property
///
class DAVIX_EXPORT FileInfoName :
    public FileInfo<FileInfoName>{
public:
    FileInfoName(const FileInfoName & orig) : name(orig.name){}
    virtual ~FileInfoName(){}
    std::string name;
};


///
/// @class FileInfo
/// \brief File access protocol
///
class DAVIX_EXPORT FileInfoProtocolType :
    public FileInfo<FileInfoProtocolType>{
public:
    FileInfoProtocolType() : protocol(){}
    FileInfoProtocolType(const FileInfoProtocolType & orig) : protocol(orig.protocol){}
    FileInfoProtocolType(const std::string & s) : protocol(s){}
    virtual ~FileInfoProtocolType(){}
    std::string protocol;
};



///
/// @typedef Properties
/// \brief File properties container
///
typedef std::vector<FileInfoInterface*> Properties;



///
/// @struct Replica
/// @brief Replica of a resource
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

} // Davix


#endif // DAVIX_FILE_INFO_HPP
