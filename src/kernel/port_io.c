#include <raam/port_io.h>

/* output byte AL to immediate port number */ 
void outportb(uint16_t port_id, uint8_t value)                                 
{                                                                              
        asm volatile("out dx, al": :"d" (port_id), "a" (value));               
}                                                                              
                                                                               
/* input byte from port DX into AL */                                          
uint8_t inportb(uint16_t port_id)                                             
{                                                                              
        uint8_t ret;                                                           
        asm volatile("in al, dx":"=a"(ret):"d"(port_id));                    
        return ret;                                                            
}
