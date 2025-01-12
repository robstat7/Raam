#ifndef PRINTK_H
#define PRINTK_H

#include <stdarg.h>
#include <raam/tty.h>
#include <lib/string.h>

// states to use
#define NORMAL			0
#define FORMAT_SPECIFIER	1

#define INT_MAX_CHARS		12 // INT_MAX = 2147483647 (for 32-bit Integers)

void printk(const char *format, ...);
static void print_arg(const char *specifier, va_list *args);

#endif	// PRINTK_H
