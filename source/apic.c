/*
 * APIC (in x2APIC mode) interrupt controller driver
 */
#include "printk.h"
#include "msr_io.h"

void enable_apic(void)
{
	uint32_t cpu_edx, cpu_eax;

	asm("cli");

	/* set the spurious interrupt vector register bit 8 to start receiving interrupts */
	read_msr_reg(0x80f, &cpu_edx, &cpu_eax);

	cpu_eax |= 0x100;

	write_msr_reg(0x80f, &cpu_edx, &cpu_eax);

	printk("@apic: started receiving interrupts!\n");


	/* set task priority to 0 to allow all */
	read_msr_reg(0x80f, &cpu_edx, &cpu_eax);

	cpu_eax &= 0xffffff00;

	write_msr_reg(0x80f, &cpu_edx, &cpu_eax);

	asm("sti");
}
