#pragma once
#ifndef DAVIX_DAVIXURI_H
#define DAVIX_DAVIXURI_H

#include <string>

namespace Davix {

class UriPrivate;

class Uri
{
public:
    Uri();
    Uri(const std::string & uri_string);
    virtual ~Uri();

    inline const std::string & getString(){
        return uri_string;
    }

protected:
    std::string uri_string;
    UriPrivate* d_ptr;
};

} // namespace Davix

#endif // DAVIX_DAVIXURI_H
