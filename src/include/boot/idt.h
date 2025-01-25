#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define MAX_IDT_GATES			256

struct idt_desc_struct {                                                                
  uint16_t limit;                                                               
  struct idt_gate_desc_struct *base;         
}__attribute__((packed));

struct idt_gate_desc_struct {                                                              
  uint16_t base_low;                                                            
  uint16_t sel;                                                                 
  uint8_t always0;                                                              
  uint8_t flags;                                                                
  uint16_t base_middle;                                                         
  uint32_t base_high;                                                           
  uint32_t zero;                                                                
}__attribute__((packed)); 

void init_idt(void);
void idt_set_gate(int index, uint64_t base, uint16_t sel, uint8_t flags);

#endif	/* IDT_H */
