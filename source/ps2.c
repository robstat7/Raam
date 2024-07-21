/*
 * PS/2 controller driver.
 */
#include "ps2.h"
#include <iso646.h>
#include <stdint.h>

uint64_t timer_count = 10000;		/* timer count */

/* PS/2 controller initialization */
int init_ps2_controller(void)
{
	setup_timer();

	/* disable devices */

	if(send_bytes_to_dev(0x64, 0xad) == 1)
		return 1;

	if(send_bytes_to_dev(0x64, 0xa7) == 1)
		return 1;

	inportb(0x60);	/* flush the output buffer */

	/* set the controller configuration byte */
	if(send_bytes_to_dev(0x64, 0x20) == 1)	/* read the configuration byte */
		return 1;

	uint8_t cc_byte = inportb(0x60);

	printk("@cc_byte = {p}\n", (void *) cc_byte);

	cc_byte &= 0xbc;	/* disable all IRQs and disable translation (clear bits 0, 1 and 6) */

	if(send_bytes_to_dev(0x64, cc_byte) == 1)
		return 1;

	return 0;
}

void setup_timer(void)
{
	uint32_t t_mode = 0x0;	/* one shot timer mode */

	/* set timer divide value */
	set_divide_value(0xb); /* divide by 1 (111) */

	set_mode(t_mode);	/* set timer mode */ 
}

int send_bytes_to_dev(uint16_t port_id, uint8_t value)
{
	int flag = 1;

	set_initial_count(timer_count);	/* set initial count of timer */

	while(read_current_count() > 0) {
		/* Poll bit 1 of the Status Register ("Input buffer empty/full") until it becomes clear */
		if((inportb(0x64) & 0x2) == 0) {
			flag = 0;
			break;
		}
	}

	if(flag == 1)
	{
		printk("@kbd.c: error: time-out expired while checking for clear status of the bit #1 of status register!\n");
	} else {
		outportb(port_id, value);
	}

	return flag;
}
