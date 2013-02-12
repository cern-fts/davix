
#include <string.h>
#include <assert.h>
//
// derivative from glib implementation
//
size_t
strlcpy (char       *dest,
           const char *src,
           size_t        dest_size)
{
  register char *d = dest;
  register const char *s = src;
  register size_t n = dest_size;

  assert(dest != NULL);
  assert(src  != NULL);

  /* Copy as many bytes as will fit */
  if (n != 0 && --n != 0)
    do
      {
        register char c = *s++;

        *d++ = c;
        if (c == 0)
          break;
      }
    while (--n != 0);

  /* If not enough room in dest, add NUL and traverse rest of src */
  if (n == 0)
    {
      if (dest_size != 0)
        *d = 0;
      while (*s++)
        ;
    }

  return s - src - 1;  /* count does not include NUL */
}

