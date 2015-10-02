#pragma once
/** Alibxx is a simple library for CXX conveniences functions
  This library is under public domain: feel free to reuse, include and change the license

  Author: Devresse Adrien
*/


// define A_LIB_NAMESPACE to empty if not define
// trigger compilation error if not defined properly
#ifndef A_LIB_NAMESPACE
#error "A_LIB_NAMESPACE need to be defined"
#endif


// pointer helpers
#include "ptr/unique.hpp"

// algorithm helpers
#include "algorithm/algorithm.hpp"

// type conversion
#include "typeconv/typeconv.hpp"

// containers
#include "containers/cache.hpp"

// chrono
#include "chrono/timepoint.hpp"


// str
#include "str/format.hpp"


// crypto
#include "crypto/base64.hpp"
#include "crypto/hmacsha.hpp"

