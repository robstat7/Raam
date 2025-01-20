#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

char* citoa(int num, char* str);
void reverse(char str[], int length);
int strncmp(const char * s1, const char * s2, size_t n);
char *strncpy(char *dst, const char *src, size_t n);
void integer_to_hex_string(uint64_t num, char *str);
int strlen(char *str);

#endif	// STRING_H
