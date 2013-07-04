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


// global file descriptor declaration
typedef struct Davix_dir_handle DAVIX_DIR;
typedef struct Davix_fd DAVIX_FD;

struct Davix_fd;
struct Davix_dir_handle;


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


/// @enum advise_t
/// Information about the next type of operation executed
/// AdviseAuto : default operation, no optimization
/// AdviseSequentialRead : optimize next operation for sequential read/write
/// AdviseRandomRead: optimize next operation for random position read/write
enum DAVIX_EXPORT advise_t{
    AdviseAuto=0x00,
    AdviseSequential,
    AdviseRandom,

};

} // Davix


#endif // DAVIX_FILE_TYPES_HPP
