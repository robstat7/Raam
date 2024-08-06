/*
 * real-time clock driver
 */
#include "rtc.h"
#include <stdint.h>

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

uint8_t get_hour()
{
	outportb(0x70, (NMI_disable_bit << 7) | (HOUR));
	uint8_t h = inportb(0x71);
	int b = (h/16) * 10 + (h & 0xf);	/* bcd mode is set */
	return (uint8_t) b;
}

uint8_t get_minute()
{
	outportb(0x70, (NMI_disable_bit << 7) | (MINUTE));
	uint8_t m = inportb(0x71);
	int b = (m/16) * 10 + (m & 0xf);	/* bcd mode is set */
	return (uint8_t) b;
}

uint8_t read_status_reg_b(void)
{
	outportb(0x70, (NMI_disable_bit << 7) | (StatusRegisterB));
	uint8_t b = inportb(0x71);
	return b;
}

