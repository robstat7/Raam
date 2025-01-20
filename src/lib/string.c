#include <lib/string.h>

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
        str[i++] = rem + '0';                    
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

// convert integer number to hexadecimal string. `str` contains the
// result.
// Implemented simple paper-pen method to convert decimal to hex number.
void integer_to_hex_string(uint64_t num, char *str)                                   
{                                                                               
	const int base = 16;	// hexadecimal base
	int i = 0;

	if(num == 0) {
		str[i++] = '0';	
	} else {
		while(num != 0) {
			int r = num % base;
			if (r < 10) {
				str[i++] = r + 48;	// '0' to '9'	
			} else {
				str[i++] = r + 87;	// 'a' to 'f'
			}

			num = num / base;
		}
	}

	// append hex notation "0x" in reverse order
	str[i++] = 'x';
	str[i++] = '0';
	// append null character
	str[i] = '\0';

	// reverse the string
	reverse(str, i);
}

int strlen(char *str)                                                           
{                                                                               
int len = 0;                                                                    
for(int i = 0; i < 10000000; i++)                                               
        if(str[i] == '\0')                                                      
                break;                                                          
        else                                                                    
                len++;                                                          
                                                                                
return len;                                                                     
}
