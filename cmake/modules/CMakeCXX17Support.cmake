include(CheckCXXSourceCompiles REQUIRED)
include(CheckCXXCompilerFlag)

# Check C++17 Support
check_cxx_compiler_flag(-std=c++17 HAVE_CXX017_FULL_SUPPORT)

if (NOT HAVE_CXX017_FULL_SUPPORT)
  message(FATAL_ERROR "Compiler with C++17 features (-std=c++17) is required!")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check TR1 Support
CHECK_CXX_SOURCE_COMPILES("
#include <tr1/functional>
int main() { return 0; }"
 HAVE_TR1_SUPPORT)

if(HAVE_TR1_SUPPORT)
  message(STATUS "TR1 support detected")
else()
  message(STATUS "no TR1 support")
endif()
