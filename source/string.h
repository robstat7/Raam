#ifndef STRING_H
#define STRING_H

#include <stddef.h>

char *strncpy(char *destination, const char *source, size_t size);
int strncmp(const char *str1, const char *str2, size_t count);

#endif /* STRING_H */
