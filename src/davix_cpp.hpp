#ifndef DAVIX_CPP_HPP
#define DAVIX_CPP_HPP



/**
  @file davix_cpp.hpp
  @author Devresse Adrien


  @brief C++ API of Davix

  Davix is a file access and file management library on top of HTTP/Webdav

  Non exhaustive list of features :
  - all commons POSIX file operations : open/read/write/close, opendir, readdir, mkdir
  - SSL client side credentials
  - Third party copy file
  - session reuse
 */


#include <davixcontext.hpp>
#include <httprequest.hpp>
#include <davixrequestparams.hpp>
#include <davixuri.hpp>
#include <posix/davposix.hpp>
#include <status/davixstatusrequest.hpp>



#endif // DAVIX_CPP_HPP
