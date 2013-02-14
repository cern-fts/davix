#include <config.h>
#include "simple_get_pass.h"

#include <iostream>

#if defined HAVE_TERMIOS_H
#include <termios.h>
#elif !defined HAVE_GETPASS
#error "impossible to compile simple_get_pass"
#endif

int simple_get_pass(char* passwd, size_t max_size){
    int ret = 0;
#ifdef HAVE_TERMIOS_H
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

#else
    char* p;
    if((p = getpass("")) == NULL)
        return -1;
    *passwd = strdup(p);
    ret = strlen(p);
#endif
    return ret;

}




void decimate_passwd(char* p){
    int i=0;
    const int s_size = strlen(p);
    for( i =0; i < s_size;i++)
            p[i] = '0';
}
