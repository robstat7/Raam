/*
 * real-time clock driver
 */
#include "port_io.h"

#define NMI_disable_bit 1
#define DAY_OF_MONTH 0x07
#define MONTH 0x08

uint8_t read_cmos_reg(void)
{
	outportb(0x70, (NMI_disable_bit << 7) | (DAY_OF_MONTH));
	uint8_t dom = inportb(0x71);

	printk("@dom = {d}\n", dom);
}
