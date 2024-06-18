#include "string.h"

int strncmp(const char * s1, const char * s2, size_t n)
{
    while (n && *s1 && (*s1 == *s2))
    {
        ++s1;
        ++s2;
        --n;
    }
    if ( n == 0 )
    {
        return 0;
    }
    else
    {
        return ( *(unsigned char *)s1 - *(unsigned char *)s2 );
    }
}

char *strncpy(char *dst, const char *src, size_t n)
{
   int i;
   char *temp;

   temp = dst;  
   for (i = 0; i < n; i++)
      *dst++ = *src++;
   return temp;
}
