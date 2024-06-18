/*
 * interrupt handlers
 */
#include "isr.h"
#include "printk.h"
#include "msr_io.h"
#include <stdint.h>

isr_t interrupt_handlers[256];

void isr_handler(registers_t regs)
{
	if (interrupt_handlers[regs.int_no] != 0) {
		isr_t handler = interrupt_handlers[regs.int_no];
		handler(regs);
	}
}

void timer_handler(registers_t regs)
{
	uint32_t cpu_edx = 0x0, cpu_eax = 0x0;

	printk("interrupt: timer: recieved interrupt: {d}\n", regs.int_no);
	write_msr_reg(0x80b, &cpu_edx, &cpu_eax);
}


void register_interrupt_handler(uint8_t n, isr_t handler) {
  interrupt_handlers[n] = handler;
}
