/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#ifndef DAVIX_INTERNAL_HPP
#define DAVIX_INTERNAL_HPP


// configuration
#include <davix_internal_config.hpp>

#include <davix.hpp>

// C++ includes
#include <sstream>
#include <ostream>
#include <iostream>
#include <memory>
#include <string>
#include <cctype>
#include <vector>
#include <queue>
#include <deque>
#include <map>
#include <iterator>
#include <functional>
#include <algorithm>
#include <limits>
#include <utility>

// C includes
#include <cstddef>
#include <cstring>
#include <cstddef>
#include <ctime>
#include <cmath>
#include <cstdlib>

// Posix includes
#include <fcntl.h>

#include "libs/alibxx/alibxx.hpp"

namespace Davix{

// http request internals
void httpcodeToDavixException(int code, const std::string & scope, const std::string & end_message = std::string());
bool httpcodeIsValid(int code);
void httpcodeToDavixError(int code, const std::string & scope, const std::string & end_message, DavixError** err);
}


#endif // DAVIX_INTERNAL_HPP
