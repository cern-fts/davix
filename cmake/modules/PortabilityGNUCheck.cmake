#
# This module setup common portability variables


INCLUDE (CheckIncludeFiles)
INCLUDE (CheckFunctionExists) 
INCLUDE (CheckSymbolExists)

##  C func
#CHECK_INCLUDE_FILES (malloc.h HAVE_MALLOC_H)
CHECK_INCLUDE_FILES (locale.h HAVE_LOCALE_H)
CHECK_INCLUDE_FILES(malloc.h HAVE_MALLOC_H)

## POSIX
CHECK_FUNCTION_EXISTS(strptime HAVE_STRPTIME_H)


## BSD
CHECK_FUNCTION_EXISTS(strlcpy HAVE_STRLCPY_H)

##GNU EXT
CHECK_FUNCTION_EXISTS(mempcpy HAVE_MEMPCPY_H)
