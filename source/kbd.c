/*
 * Keyboard driver.
 */
#include "printk.h"
#include "port_io.h"

int init_keyboard_driver(void)
{
	detect_keyboard();

	return 0;
}

int detect_keyboard(void)
{
	/* Send the "disable scanning" command 0xF5 to the device */
	outportb(0x64, 0xF5);

	/* check for the device to send "ACK" back (0xFA) */
	while(inportb(0x60) != 0xfa);

	printk("@ps2: device acknowledged disable scanning!\n");

	/* Send the "identify" command 0xF2 to the device */
	outportb(0x64, 0xf2);

	// Explicit memory barrier for use with GCC.
	// asm volatile ("": : :"memory");

	/* Wait for the device to send "ACK" back (0xFA) */
	while(inportb(0x60) != 0xfa);

	printk("@ps2: identify command acknowledged!\n");


	/* Wait for the device to send up to 2 bytes of reply */

	/* set timer divide value */
	set_divide_value(0xb); /* divide by 1 (111) */

	uint32_t mode = 0x0;	// one shot timer mode
	uint64_t initial_cnt = 120000;

	set_mode(mode); 

	set_initial_count(initial_cnt);	/* set initial count of timer */


	while(read_current_count() < 100000) {
		if(inportw(0x60) >= 0x0) {
			printk("ps2: error: couldn't detect Ancient AT keyboard!\n");
			return 1;
		}
	}

	printk("@ps2: Ancient AT keyboard detected!\n");


	return 0;	/* success */
}
