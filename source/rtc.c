/*
 * real-time clock driver
 */
#include "rtc.h"

uint8_t get_day_of_month()
{
	outportb(0x70, (NMI_disable_bit << 7) | (DAY_OF_MONTH));
	uint8_t dom = inportb(0x71);
	return dom;
}

uint8_t get_month()
{
	outportb(0x70, (NMI_disable_bit << 7) | (MONTH));
	uint8_t m = inportb(0x71);
	return m;
}
