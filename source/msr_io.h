#include <stdint.h>
 
void write_msr_reg(uint32_t reg_address, uint32_t *cpu_edx, uint32_t *cpu_eax);
void read_msr_reg(uint32_t reg_address, uint32_t *cpu_edx, uint32_t *cpu_eax);
