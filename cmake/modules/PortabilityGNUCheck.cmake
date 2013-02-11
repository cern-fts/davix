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
CHECK_SYMBOL_EXISTS(strptime time.h HAVE_STRPTIME_H)



##GNU EXT
CHECK_SYMBOL_EXISTS(mempcpy string.h HAVE_MEMPCPY_H)
