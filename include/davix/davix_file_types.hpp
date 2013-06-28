#ifndef DAVIX_FILE_TYPES_HPP
#define DAVIX_FILE_TYPES_HPP

#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <davix_types.h>

/**
  @file davix_file_types.hpp
  @author Devresse Adrien


  @brief davix file related type declarations
*/

namespace Davix{

/// @struct DavIOVecInput
/// @brief input parameters for vector operations in Davix
struct DAVIX_EXPORT DavIOVecInput{
    void* diov_buffer;                    /**< buffer, in case of read : destination buffer, in case of write : source buffer */
    dav_off_t diov_offset;                /**< initial offset taken from the source */
    dav_size_t diov_size;                 /**< size of the data requested */
};

/// @struct DavIOVecOuput
/// @brief result of vector operations in Davix
struct DAVIX_EXPORT DavIOVecOuput{
    void* diov_buffer;                    /**< pointer to the buffer used for this fragment */
    dav_ssize_t diov_size;                /**< size of the data returned, -1 if error */
};


enum DAVIX_EXPORT advise_t{
    Auto=0x00,
    SequentialRead,
    RandomRead

};

} // Davix


#endif // DAVIX_FILE_TYPES_HPP
