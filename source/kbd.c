/*
 * Keyboard driver.
 */

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

	printk("@ps2: device sent ACK back!\n");

	return 0;	/* success */
}
