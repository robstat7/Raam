/*
 * MSR register read and write functions.
 */
#include "msr_io.h"
#include <stdint.h>
 
void write_msr_reg(uint32_t reg_address, uint32_t *cpu_edx, uint32_t *cpu_eax)
 {
 	__asm__("mov ecx, %0\n\t"
		"mov edx, %1\n\t"
		"mov eax, %2\n\t"
		"wrmsr"
		::"m" (reg_address),
 		"m" (*cpu_edx),
		"m" (*cpu_eax):"ecx", "edx", "eax");
}

void read_msr_reg(uint32_t reg_address, uint32_t *cpu_edx, uint32_t *cpu_eax)
 {
 	__asm__("mov ecx, %0\n\t"
		"rdmsr\n\t"
		"mov %1, edx\n\t"
		"mov %2, eax"
		::"m" (reg_address),
 		"m" (*cpu_edx),
		"m" (*cpu_eax):"ecx", "edx", "eax");
}
