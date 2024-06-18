/*
 * local apic (in x2apic mode) timer driver
 */
#include "printk.h"
#include "string.h"
#include "msr_io.h"
#include "isr.h"

/*
 * initialize the timer
 *
 * returns 0 if success else 1.
 */
int timer_init(void)
{
	int i;
	uint32_t cpu_edx, cpu_eax, msg1, msg2, msg3, value1, value2, value3;
	uint16_t cpuid_ax;
	char *gen_intel_msg = "GenuineIntel";

	asm("cli");

	/* check for the "GenuineIntel" message */
	__asm__("mov eax, 0\n\t"
		"cpuid\n\t"
		"mov %0, ebx\n\t"
		"mov %1, edx\n\t"
		"mov %2, ecx"
		::"m" (msg1),
		"m" (msg2),
		"m" (msg3):);
	
	for(i = 0; i < 4; i++) {
		if(strncmp(gen_intel_msg[i], (char) msg1, 1) !=0) {
			printk("error: timer: cpuid: processor is not genuine intel!\n");
			return 1;
		}
		msg1 >>= 8;
	}
	
	for(; i < 8; i++) {
		if(strncmp(gen_intel_msg[i], (char) msg2, 1) !=0) {
			printk("error: timer: cpuid: processor is not genuine intel!\n");
			return 1;
		}
		msg2 >>= 8;
	}
	
	for(; i < 12; i++) {
		if(strncmp(gen_intel_msg[i], (char) msg3, 1) !=0) {
			printk("error: timer: cpuid: processor is not genuine intel!\n");
			return 1;
		}
		msg3 >>= 8;
	}
	
	
	printk("@timer: cpuid: check for GenIntel message passed!\n");
	
	__asm__("mov eax, 0x1\n\t"
		"cpuid\n\t"
		"mov eax, 0x10\n\t"
		"mov ebx, 0x200000\n\t"
		"mov esi, edx\n\t"
		"and esi, 0x200\n\t"
		"and edx, eax\n\t"
		"and ecx, ebx\n\t"
		"mov %0, edx\n\t"
		"mov %1, ecx\n\t"
		"mov %2, esi"
		::"m" (value1),
		"m" (value2),
		"m" (value3):);
		
	/* check whether MSRs are supported */
	if (value1 == 0x10) {
		printk("@timer: cpuid: MSRs are supported!\n");
	} else if (value1 == 0x0) {
		printk("error: timer: cpuid: MSRs aren't supported!\n");
		return 1;
	}
	
	/* check whether the CPU has a built-in local APIC and if it hasn't been disabled in MSRs */
	if(value3 == 0x200) {
		printk("@timer: cpuid: the cpu has a built-in local apic!\n");
	} else {
		printk("error: timer: cpuid: either the cpu doesn't have a local apic or it has been disabled in MSRs!\n");
		return 1;
	}
	
	/* detecting x2APIC mode */
	if (value2 == 0x200000) {
		printk("@timer: cpuid: the processor supports the x2APIC capability!\n");
	} else if (value2 == 0x0) {
		printk("error: timer: cpuid: the processor doesn't support the x2APIC capability!\n");
		return 1;
	}
	
	/* enable the local apic */
	__asm__("mov ecx, 0x0\n\t"
		"mov ecx, 0x1b\n\t"
		"rdmsr\n\t"
		"or eax, 0x800\n\t"
		"wrmsr\n\t"
		"rdmsr\n\t"
		"and eax, 0x800\n\t"
		"mov %0, eax"
		::"m" (value1):);
	
	if (value1 == 0x800) {
		printk("@timer: init: enabled the local APIC!\n");
	} else {
		printk("error: timer: init: unable to enable the local APIC!\n");
		return 1;
	}
	
	/* enabling x2APIC mode */
	__asm__("mov ecx, 0x0\n\t"
		"mov ecx, 0x1b\n\t"
		"rdmsr\n\t"
		"or eax, 0x400\n\t"
		"wrmsr\n\t"
		"rdmsr\n\t"
		"and eax, 0xc00\n\t"
		"mov %0, eax"
		::"m" (value1):);
	
	if (value1 == 0xc00) {
		printk("@timer: init: enabled the local APIC in x2APIC mode!\n");
	} else {
		printk("error: timer: init: unable to enable the local APIC in x2APIC mode!\n");
		return 1;
	}
	
	
	/* set interrupt vector number to 172 */
	read_msr_reg(0x832, &cpu_edx, &cpu_eax);
	
	cpu_eax &= 0xffffff00;
	
	cpu_eax |= 0xac;
	
	write_msr_reg(0x832, &cpu_edx, &cpu_eax);
	
	
	/* unmask the timer interrupt by setting bit #16 to 0 */
	read_msr_reg(0x832, &cpu_edx, &cpu_eax);
	
	cpu_eax &= 0xfffeffff;
	
	write_msr_reg(0x832, &cpu_edx, &cpu_eax);
	
	
	/* register interrupt handler for the timer interrupt */
	register_interrupt_handler(172, timer_handler);
	
	
	/* get the lapic frequency equal to the core crystal's frequency */
	__asm__("mov eax, 0x16\n\t"
		"cpuid\n\t"
		"mov %0, ax"
		::"m" (cpuid_ax):);
	
	printk("@timer: cpuid: core base frequency is {d} MHz\n", cpuid_ax);
	
	
	asm("sti");
	
	return 0;
}

void set_divide_value(int value)
{
	__asm__("mov ecx, 0x0\n\t"
		"mov ecx, 0x83e\n\t"
		"rdmsr\n\t"
		"and eax, 0xfffffff4\n\t"
		"or eax, %0\n\t"
		"wrmsr"
		::"m" (value):);
}

/* returns 0 on success else 1 */
int set_mode(uint32_t mode)
{
	uint32_t res;

	__asm__("mov eax, 0x1\n\t"
		"cpuid\n\t"
		"and ecx, 0x1000000\n\t"
		"mov %0, ecx"
		::"m" (res):);	

	if(res == 0x1000000)
		printk("@timer: set_mode: ecx bit 24 is 1!\n");
	else if(res == 0x0) {
		printk("@timer: set_mode: ecx bit 24 is 0! not setting the mode using only the bit 17 at the moment!\n");
		return 1;
	}

	__asm__("mov ecx, 0x0\n\t"
		"mov ecx, 0x832\n\t"
		"rdmsr\n\t"
		"and eax, 0xfff9ffff\n\t"
		"or eax, %0\n\t"
		"wrmsr"
		::"m" (mode):);

	return 0;
}

void set_initial_count(uint64_t count)
{
	uint32_t mode;

	/* get the mode first */
	__asm__("mov ecx, 0x0\n\t"
		"mov ecx, 0x832\n\t"
		"rdmsr\n\t"
		"and eax, 0x60000\n\t"
		"mov %0, eax"
		::"m" (mode):"ecx", "edx", "eax");

	if(mode == 0x0 || mode == 0x20000) {	/* one-shot or periodic mode */
		printk("@timer: set_initial_count: mode is either one-shot or periodic!\n");

		__asm__("mov ecx, 0x838\n\t"
			"mov edx, 0x0\n\t"
			"mov rax, %0\n\t"	/* count must fit in eax */
			"wrmsr"			
			::"m" (count):"ecx", "edx", "rax");
	}
}

uint64_t read_current_count(void)
{
	uint32_t mode;
	
	/* get the mode first */
	__asm__("mov ecx, 0x0\n\t"
		"mov ecx, 0x832\n\t"
		"rdmsr\n\t"
		"and eax, 0x60000\n\t"
		"mov %0, eax"
		::"m" (mode):"ecx", "edx", "eax");

	 if(mode == 0x0 || mode == 0x20000) {	/* one-shot or periodic mode */
		uint32_t count;

		__asm__("mov ecx, 0x0\n\t"
			"mov ecx, 0x839\n\t"
			"rdmsr\n\t"
			"mov %0, eax"
			::"m" (count):"ecx", "edx", "eax");

		return count;
	}

}

void stop_timer(void)
{
	set_initial_count(0);
}
