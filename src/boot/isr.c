/*                                                                              
 * interrupt handlers                                                           
 */                                                                             
#include <boot/isr.h>                                                               
#include <raam/printk.h>                                                            
#include <raam/pic.h>                                                            
                                                                                
isr_t interrupt_handlers[256];                                                  
                                                                                
void isr_handler(registers_t regs)                                              
{                                                                               
        if (interrupt_handlers[regs.int_no] != 0) {                             
                isr_t handler = interrupt_handlers[regs.int_no];                
                handler(regs);                                                  
        } else {                                                                
                printk("interrupt: recieved interrupt: {d}  ", regs.int_no);   
                asm volatile("hlt": :);                                         
        }                                                                       
}                                                                               
                                                                                
void register_interrupt_handler(uint8_t n, isr_t handler) {                     
  interrupt_handlers[n] = handler;                                              
}      

void timer_handler(registers_t regs)                                            
{                                                                               
        printk("interrupt: timer: recieved interrupt: {d}  ", regs.int_no);
	/* send end of interrupt */
	pic_send_eoi(0);
}
