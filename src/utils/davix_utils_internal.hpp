#pragma once
#ifndef DAVIX_UTILS_INTERNAL_HPP
#define DAVIX_UTILS_INTERNAL_HPP

#include <davixuri.hpp>
#include <params/davixrequestparams.hpp>

namespace Davix{

void configureRequestParamsProto(const Uri & uri, RequestParams & params);

}

#endif // DAVIX_UTILS_INTERNAL_HPP
