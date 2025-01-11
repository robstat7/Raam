#include <raam/printk.h>

void printk(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int state = NORMAL;	// begin with normal state
	char *specifier;

	for(int i = 0; format[i] != '\0'; i++) {
		char current = format[i];
		switch(current) {
			case '{':
				state = FORMAT_SPECIFIER;
				break;
			case 'd':
				if(state == NORMAL) {
					tty_put_char(current);
				} else {
					specifier = "d";
				}
				break;
			case '}':
				print_arg(specifier, &args);			
				state = NORMAL;
				break;
			default:
				tty_put_char(current);
		}
	}

	va_end(args);
}

static void print_arg(const char *specifier, va_list *args)
{
	int arg;
	char str[60];

	switch(specifier[0]) {
		case 'd':
			arg = va_arg(*args, int);
			citoa(arg, str);
			printk(str);
			break;
	}
}
