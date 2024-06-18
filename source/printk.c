#include "printk.h"
#include <stdbool.h>
#include <stdint.h>
#include "string.h"

char* citoa(int num, char* str);
char* citoa_int64_t(int64_t num, char* str);
char* citoa_uint64_t(uint64_t num, char* str);
void uint64_t_to_hex(uint64_t num, char *str);

void print_arg(const char *specifier, va_list *ap)
{
	char ch;
	int arg;
	int64_t num;
	void *addr;
	uint64_t res;
	char str[60];

		switch(specifier[0]) {
			case 'c':
                                ch = va_arg(*ap, int);
                        	write_char(ch);
				break;
			case 'd':
                                arg = va_arg(*ap, int);
				citoa(arg, str);
				printk(str);
				break;
			case 'p':
                                addr = va_arg(*ap, void *);
				res = (uint64_t) ((uint64_t *) addr);
				uint64_t_to_hex(res, str);
				printk(str);
				break;
			case 'l':
				if (strncmp(specifier, "lld", 3) == 0) {	/* int64_t */
					num = va_arg(*ap, int64_t);
					citoa_int64_t(num, str);
					printk(str);
				}
				else if (strncmp(specifier, "llu", 3) == 0) {	/* uint64_t */
					res = va_arg(*ap, uint64_t);
					citoa_uint64_t(res, str);
					printk(str);
				}
				break;
		}
}

void printk(const char *format, ...)
{
	va_list ap;
	int i, spf = 0;
	int state;
	char current;
	char *specifier, spf_buff[3];

	va_start(ap, format);		

	state = NORMAL;

	for(i = 0; format[i] != '\0'; i++) {
		current = format[i];
 
		switch(current) {
		case '{':
			state = FORMAT_SPECIFIER;
			break;
		case '}':
			if (spf == 0) {			/* specifier is a single character like d and c */
				print_arg(specifier, &ap);
				state = NORMAL;
			}
			else {
				print_arg(spf_buff, &ap);
				spf = 0;
				state = NORMAL;
			}
			break;
		case 'c':
			if (state == NORMAL)
				write_char(current);
			else
				specifier = "c";
			break;
		case 'd':
			if (state == NORMAL)
				write_char(current);
			else {
				if(spf != 0)
					spf_buff[spf]='d';
				else
					specifier = "d";
			}
			break;
		case 'p':
			if (state == NORMAL)
				write_char(current);
			else
				specifier = "p";
			break;
		case 'l':
			if (state == NORMAL)
				write_char(current);
			else
				spf_buff[spf++]= 'l';
			break;
		case 'u':
			if (state == NORMAL)
				write_char(current);
			else
				spf_buff[spf]= 'u';
			break;
		default:
			write_char(current);
			break;
		}
	}

	va_end(ap);
}

// A utility function to reverse a string
void reverse(char str[], int length)
{
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        end--;
        start++;
    }
}

/*
 * convert uint64_t to string.
 */
char* citoa_uint64_t(uint64_t num, char* str)
{
    int i = 0;
    uint64_t base;
 
    base = 10;
    /* Handle 0 explicitly, otherwise empty string is
     * printed for 0 */
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
 
    // Process individual digits
    while (num != 0) {
        uint64_t rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
 
    str[i] = '\0'; // Append string terminator
 
    // Reverse the string
    reverse(str, i);
 
    return str;
}

/*
 * convert int64_t to string.
 */
char* citoa_int64_t(int64_t num, char* str)
{
    int i = 0;
    bool isNegative = false;
    int64_t base;
 
    base = 10;
    /* Handle 0 explicitly, otherwise empty string is
     * printed for 0 */
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
 
    // In standard itoa(), negative numbers are handled
    // only with base 10. Otherwise numbers are
    // considered unsigned.
    if (num < 0 && base == 10) {
        isNegative = true;
        num = -num;
    }
 
    // Process individual digits
    while (num != 0) {
        uint64_t rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
 
    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';
 
    str[i] = '\0'; // Append string terminator
 
    // Reverse the string
    reverse(str, i);
 
    return str;
}

/*
 * convert integer to string.
 */
char* citoa(int num, char* str)
{
    int i = 0;
    bool isNegative = false;
    int base;
 
    base = 10;
    /* Handle 0 explicitly, otherwise empty string is
     * printed for 0 */
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
 
    // In standard itoa(), negative numbers are handled
    // only with base 10. Otherwise numbers are
    // considered unsigned.
    if (num < 0 && base == 10) {
        isNegative = true;
        num = -num;
    }
 
    // Process individual digits
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
 
    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';
 
    str[i] = '\0'; // Append string terminator
 
    // Reverse the string
    reverse(str, i);
 
    return str;
}

void uint64_t_to_hex(uint64_t num, char *str)
{
	int i;
	int temp; 
	int ch;
	int r;

	i = 0;
    
	if(num == 0) {
		str[i++] = '0';
	} else {
		/* if decimal number is not equal to zero then enter in to the loop and
		 * execute the statements
		 */
		while (num != 0) { 
        		ch = num / 16; 
        		r = ch * 16; 
        		temp = num - r; 
        	
			/* convert decimal number in to a hexadecimal number */
			if(temp < 10) 
        			temp = temp + 48; 
        		else
        			temp = temp + 87; 

        		str[i++] = temp; 
        		num = num / 16; 
    		} 
	}

	str[i++] = 'x';
	str[i++] = '0';
	str[i] = '\0';

    	/* reverse the string */
    	reverse(str, i);
}
