#ifndef MEMORYUTILS_HPP
#define MEMORYUTILS_HPP

#include <memory>




template<typename obj_type>
struct ScopedPtr{

#if (defined HAVE_CXX011_FULL_SUPPORT) || (defined HAVE_CXX011_PARTIAL_SUPPORT)
typedef std::unique_ptr<obj_type> type;
#else
typedef std::auto_ptr<obj_type> type;
#endif

};

#endif // MEMORYUTILS_HPP
