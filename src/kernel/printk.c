#include <raam/printk.h>

/*
 * printk
 *
 * This function prints the passed format string and the required
 * arguments to the terminal. It can be called as follows:
 *
 *		printk("Raam Raam sa");
 *
 * And with a format specifier as follows:
 *
 *		printk("error: couldn't find the controller."
		       "Error code is {d}.", code);
 *
 * This is a variadic function. It also changes the `state` from normal
 * to format specifier and back to print the variable arguments.
 */
void printk(const char *format, ...)
{
	va_list args;
	va_start(args, format);	// first, initialize the va_list
	int state = NORMAL;	// begin with normal state
	char *specifier;	// will contain the specifier letter

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

	va_end(args);	// clean up the va_list
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
