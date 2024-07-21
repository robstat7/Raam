/*
 * Keyboard driver.
 */
#include "ps2.h"

int detect_ps2dev(void);

int init_keyboard_driver(void)
{
	if(detect_ps2dev() == 1) {
		return 1;
	}

	printk("@kbd.c : init: keyboard detected!\n");

	return 0;
}

int detect_ps2dev(void)
{	
	
	printk("@kbd.c: detecting ps/2 device...\n");
	printk("@kbd.c: status reg value = {p}\n", inportb(0x64));


	/* Send the "disable scanning" command 0xF5 to the device */

	if(send_bytes_to_dev(t_cnt, 0x60, 0xf5) == 1)
		return 1;

	/* TODO: receive byte from device using IRQ */

	// /* check for the device to send "ACK" back (0xFA) */
	// while(inportb(0x60) != 0xfa);

	// printk("@ps2: device: acknowledged disable scanning!\n");

	// /* Send the "identify" command 0xF2 to the device */
	// outportb(0x60, 0xf2);

	// /* Wait for the device to send "ACK" back (0xFA) */
	// while(inportb(0x60) != 0xfa);	

	// printk("@ps2: device: identify command acknowledged!\n");

	// /* Wait for the device to send up to 2 bytes of reply */

	// set_initial_count(initial_cnt);	/* set initial count of timer */

	// printk("@ps2: device: {p}\n", (void *) inportb(0x60));
	// printk("@ps2: device: {p}\n", (void *) inportb(0x60));

	// while(read_current_count() < 100000) {
	// 	if(inportb(0x60) >= 0x0) {
	// 		printk("ps2: error: couldn't detect Ancient AT keyboard!\n");
	// 		return 1;
	// 	}
	// }

	// inportb(0x60);

	// printk("@ps2: Ancient AT keyboard detected!\n");


	// /* Send the "enable scanning" command 0xF4 to the device */
	// outportb(0x60, 0xf4);

	// /* check for the device to send "ACK" back (0xFA) */
	// while(inportb(0x60) != 0xfa);

	// printk("@ps2: device: enable scanning command acknowledged!\n");

	return 0;	/* success */
}
