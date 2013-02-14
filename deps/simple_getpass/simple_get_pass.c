#include <config.h>
#include "simple_get_pass.h"

#if defined HAVE_TERMIOS_H
#include <termios.h>
#elif !defined HAVE_GETPASS
#error "impossible to compile simple_get_pass"
#endif

#endif

ssize_t simple_get_pass(char** passwd){
    ssize_t ret = 0;
#ifdef HAVE_TERMIOS_H
    FiLE* stream =stdin;
    struct termios old, new;
    int nread;
    *passwd = NULL;

    /* Turn echoing off and fail if we can't. */
    if (tcgetattr (fileno (stream), &old) != 0)
      return -1;
    new = old;
    new.c_lflag &= ~ECHO;
    if (tcsetattr (fileno (stream), TCSAFLUSH, &new) != 0)
      return -1;

    /* Read the password. */
    ret =  (ssize_t)  getline (passwd, NULL, stream);

    /* Restore terminal. */
    (void) tcsetattr (fileno (stream), TCSAFLUSH, &old);

    returnret;
#else
    char* p;
    if((p = getpass("")) == NULL)
        return -1;
    *passwd = strdup(p);
    ret = strlen(p);
#endif
    return ret;

}
