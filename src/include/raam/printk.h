#ifndef PRINTK_H
#define PRINTK_H

#include <stdarg.h>
#include <lib/string.h>

// states to use
#define NORMAL			0
#define FORMAT_SPECIFIER	1

void printk(const char *format, ...);
static void print_arg(const char *specifier, va_list *args);

#endif	// PRINTK_H
