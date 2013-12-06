#ifndef DAVIX_FILE_INFO_HPP
#define DAVIX_FILE_INFO_HPP


#include <davix_file_types.hpp>
#include <davixuri.hpp>

/**
  @file davix_file_info.hpp
  @author Devresse Adrien

  @brief davix file Meta-data class
*/

namespace Davix{

///
/// \brief Generic container Traits
///
///  Base struct for any generic
///
class DAVIX_EXPORT FileInfoInterface : NonCopyable{
public:
    virtual ~FileInfoInterface(){}

    virtual const std::type_info & getType() =0;
};


template <class T>
class DAVIX_EXPORT FileInfo : public FileInfoInterface{
public:
    virtual ~FileInfo(){}

    virtual const std::type_info & getType(){
        return typeid(*static_cast<T*>(this));
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
class DAVIX_EXPORT Replica: NonCopyable{
public:
    inline Replica() :
        uri(),
        props(){}

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
