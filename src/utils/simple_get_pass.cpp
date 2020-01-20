/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) 2013  Adrien Devresse <adrien.devresse@cern.ch>, CERN
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


#include <davix_internal_config.hpp>
#include "simple_get_pass.h"
#include <cstdio>
#include <iostream>

#if defined HAVE_TERMIOS_H
#include <termios.h>
#elif defined HAVE_GETPASS
#include <unistd.h>
#elif defined HAVE_SETCONSOLEMODE
#include <windows.h>
#else
#error "impossible to compile simple_get_pass"
#endif

int simple_get_pass(char* passwd, size_t max_size){
    int ret = 0;
#if defined HAVE_TERMIOS_H
    struct termios old_term, new_term;
    FILE* stream = stdin;

    /* Turn echoing off and fail if we can't. */
    if (tcgetattr (fileno (stream), &old_term) != 0)
      return -1;
    new_term = old_term;
    new_term.c_lflag &= ~ECHO;
    if (tcsetattr (fileno (stream), TCSAFLUSH, &new_term) != 0)
      return -1;

    /* Read the password. */
    std::cin.getline(passwd, max_size);
    ret = strlen(passwd);

    /* Restore terminal. */
    (void) tcsetattr (fileno (stream), TCSAFLUSH, &old_term);

#elif HAVE_SETCONSOLEMODE
	HANDLE hstdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;

	if (!GetConsoleMode(hstdin, &mode))
		return -1;

	if (hstdin == INVALID_HANDLE_VALUE || !(SetConsoleMode(hstdin, 0)))
		return -1;
    std::cin.getline(passwd, max_size);
    ret = strlen(passwd);

	if (!SetConsoleMode(hstdin, mode))
		return -1;
#else
    char* p;
    if((p = getpass("")) == NULL)
        return -1;
    *passwd = strdup(p);
    ret = strlen(p);
#endif
    return ret;

}



