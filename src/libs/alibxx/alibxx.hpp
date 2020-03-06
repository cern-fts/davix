#pragma once
/** Alibxx is a simple library for CXX conveniences functions
  This library is under public domain: feel free to reuse, include and change the license

  Author: Devresse Adrien
*/


// define A_LIB_NAMESPACE to Davix by default
#ifndef A_LIB_NAMESPACE
#define A_LIB_NAMESPACE Davix
#endif


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

