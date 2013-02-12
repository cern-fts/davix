#pragma once
#ifndef _STRLCPY_H_
#define _STRLCPY_H_
/* Convert a string representation of time to a time value.
   Copyright (C) 1996, 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Devresse Adrien adev88@gmail.com>, 2012.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */


#ifndef HAVE_STRLCPY_H


#include <config.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif




size_t strlcpy (char *dest, const char *src, size_t dest_size);


#ifdef __cplusplus
}
#endif


#endif // HAVE_STRLCPY_H

#endif // _STRLCPY_H_
