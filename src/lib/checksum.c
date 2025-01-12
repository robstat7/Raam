#include <lib/checksum.h>

/*                                                                              
 * validate_checksum                                                            
 *                                                                              
 * This function sums all the bytes of the table and returns the lowest         
 * byte of the sum. It returns 0 on success.                                    
 */                                                                             
uint8_t validate_checksum(const uint8_t *table, uint32_t length)         
{                                                                               
        uint32_t sum = 0;                                                       
                                                                                
        for(uint32_t i = 0; i < length; i++) {                                  
                sum += table[i];                                                
        }                                                                       
                                                                                
        return (uint8_t) sum;                                                   
}
