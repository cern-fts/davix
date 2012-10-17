#pragma once
#ifndef DAVIX_DAVIXURI_HPP
#define DAVIX_DAVIXURI_HPP

#include <string>
#include <status/davixstatusrequest.hpp>


/**
  @file davixuri.hpp
  @author Devresse Adrien

  @brief C++ URI utilities functions for davix
 */


namespace Davix {

struct UriPrivate;

// convenience class for uri parsing
class Uri
{
public:
    Uri();
    Uri(const std::string & uri_string);
    virtual ~Uri();

    /// get a string representation of the uri
    const std::string & getString() const;

    /// return the port number
    int getPort() const;

    const std::string & getProtocol() const;

    const std::string & getHost() const;

    const std::string & getPath()const;

    StatusCode::Code getStatus() const;

protected:
    std::string uri_string;
    UriPrivate* d_ptr;
    StatusCode::Code code;
    void _init();
};

} // namespace Davix

#endif // DAVIX_DAVIXURI_HPP
