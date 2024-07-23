/*
 * PS/2 controller driver.
 */
#include "ps2.h"
#include <stdint.h>

uint64_t timer_count = 50000;		/* timer count */

/* PS/2 controller initialization */
int init_ps2_controller(uint64_t *xsdp)
{
	setup_timer();

	uint64_t *madt = get_madt_pointer(xsdp);

	if(madt == NULL)
		return 1;

	uint8_t value = *(volatile uint8_t *) ((char *) madt + 52 );



	printk("@ioapic value = {d}\n", value);



	/* disable devices */

	if(send_bytes_to_dev(0x64, 0xad) == 1)
		return 1;

	if(send_bytes_to_dev(0x64, 0xa7) == 1)
		return 1;

	inportb(0x60);	/* flush the output buffer */


	/* read and set the controller configuration byte */
	
	uint8_t cc_byte = read_controller_configuration_byte();

	printk("@contr 1 cc_byte = {p}\n", (void *) cc_byte);

	int dual_channel_controller = is_dual_channel_controller(cc_byte);

	printk("@contr 1 dual_channel_controller = {d}\n", dual_channel_controller);

	cc_byte &= 0xbc;	/* disable all IRQs and disable translation (clear bits 0, 1 and 6) */

	printk("@contr 1 new cc_byte = {p}\n", (void *) cc_byte);

	if(send_bytes_to_dev(0x64, 0x60) == 1)
		return 1;

	if(send_bytes_to_dev(0x64, cc_byte) == 1)
		return 1;

	/* determine if there are 2 channels */
	if(dual_channel_controller == 1) {
		if(send_bytes_to_dev(0x64, 0xa8) == 1)	/* enable the second PS/2 port */
			return 1;

		printk("@ps2: second PS/2 port is enabled!\n");

		uint8_t cc_byte = read_controller_configuration_byte();
		printk("@contr 2 cc_byte = {p}\n", (void *) cc_byte);
		int dual_channel_controller = !is_dual_channel_controller(cc_byte);
		printk("@contr 2 dual_channel_controller = {d}\n", dual_channel_controller);

		if(dual_channel_controller) {
			if(send_bytes_to_dev(0x64, 0xa7) == 1)	/* disable the second PS/2 port */
				return 1;

			printk("@ps2: second PS/2 port disabled!\n");
			printk("@ps2: only 1 channel is present!\n");
		} else {
			printk("@ps2: 2 channels are present!\n");
		}
	}

	/* enable the first port */

	if(send_bytes_to_dev(0x64, 0xae) == 1)
			return 1;

	printk("@ps2: first port enabled!\n");


	/* enable interrupts */

	if(dual_channel_controller)
		cc_byte |= 0x3;	/* set bit 0 and 1 */
	else
		cc_byte |= 0x1; /* set bit 0 */

	if(send_bytes_to_dev(0x64, 0x60) == 1)
			return 1;

	if(send_bytes_to_dev(0x64, cc_byte) == 1)
			return 1;

	printk("@ps2: interrupts enabled!\n");


	/* reset devices */

	if(send_bytes_to_dev(0x60, 0xff) == 1)
		return 1;

	/*
	 * while(inportb(0x60) != 0xfa);
	 *
	 * if(dual_channel_controller) {
	 *	 if(send_bytes_to_dev(0x64, 0xd4) == 1)
	 *		 return 1;
	 *
	 *	 if(send_bytes_to_dev(0x60, 0xff) == 1)
	 *		return 1;
	 *
	 *	 while(inportb(0x60) != 0xfa);
	 * }
	 */

	printk("@ps2: device(s) reset completed!\n");

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
		printk("@ps2: error: time-out expired while checking for clear status of the bit #1 of status register!\n");
	} else {
		outportb(port_id, value);
	}

	return flag;
}

/* read the configuration byte */
uint8_t read_controller_configuration_byte(void)
{
	if(send_bytes_to_dev(0x64, 0x20) == 1)
		return 1;

	return inportb(0x60);
}

/* 
 * check bit 5 of controller configuration byte to determine if it is a "dual channel" controller.
 * set = true
 * clear = false
 */
int is_dual_channel_controller(uint8_t cc_byte)
{
	int dual_channel_controller = 0;

	if((cc_byte & 0x20) == 0x20)
		dual_channel_controller = 1;

	return dual_channel_controller;
}
