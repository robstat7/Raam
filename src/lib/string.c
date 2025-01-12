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
