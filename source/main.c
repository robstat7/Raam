/*
 * Raam Raam sa _/\_ _/\_ _/\_
 *
 * Core's main function
 */
#include <stdint.h>
#include "printk.h"
#include "ps2.h"

int main(void *xsdp, uint8_t *sys_var_ptr)
{
	printk("_/\\_ Welcome to Raam Computer Software System x86-64 version 0.02 _/\\_");
	printk("\n");
	printk("  ");
	printk("\n");
	printk("  ");
	printk("\n");
	printk("  ");
	printk("\n");
	printk("Raam Raam sa _/\\_ _/\\_ _/\\_");
	printk("\n  \n  \n  \n");

	printk("Type \"Raam\" name in capitalized case and press enter to continue ...");
	printk("\n  \n  \n");

	printk("_ ");


 	/* initialize the timer */
 	 if(timer_init() == 1)
 	 	goto end;
 	
 	/* enable APIC interrupt controller */
 	enable_apic();

	 /* initialize the PS/2 controller */
	if(init_ps2_controller(xsdp) == 1)
		goto end;
	
	/* initialize the keyboard driver */
	if(init_keyboard_driver() == 1)
		goto end;

	uint8_t key;
	uint8_t last_key_pressed = 0x0;
	char key_char;

	/* scan code set is 1 */
	while(1) {
		key = inportb(0x60);

		/* if( key != last_key_pressed || key == 0xfa || key == 0xab || key == 0xaa || key == 0x0)
		 *	continue;
		 */

		if(key == last_key_pressed)
			continue;


		else if(key == 0x13 && last_key_pressed == 0x2a) /* r key pressed with left shift */
			key_char = 'R';

		else if(key == 0x1e)
			key_char = 'a';

		else if(key == 0x32)
			key_char = 'm';

		else if(key == 0x39)
			key_char = ' ';

		else if(key == 0x1c)
			key_char = 0xa;	/* enter key */
		else if(key == 0x0e)
			key_char = 0x8;	/* backspace */

		else
		{
			last_key_pressed = key;
			continue;
		}

		printk("{c}", key_char);

		last_key_pressed = key;
	}

	/* print the command prompt */
	// printk("# _");

end:
	/* hang here */
	for(;;);

	return 1;
}
