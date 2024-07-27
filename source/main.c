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
	/* initialize the global memory manager */
	// init_global_mm();	
	// mm_init();


		// printk("Raam Raam sa\n");


	// put_red_pixel();

//	__asm__ volatile ("mov $60, %eax; mov $0, %edi; syscall "); 
	

	// int b = 5/0;

	// int * a = NULL;
	// 
	// *a = 5;

	

 	/* initialize the timer */
 	 if(timer_init() == 1)
 	 	goto end;
 	
 	/* enable APIC interrupt controller */
 	enable_apic();
// 
// 
// 	// /* call test timer function */
// 	test_timer();
// 	
// 	
// 	/* initialize the xhci host controller */
// 	if(xhci_init(xsdp, sys_var_ptr) == 1)
// 		goto end;
// 
// 
// 	/* init nvme */
// 	if(nvme_init(xsdp, sys_var_ptr) == 1)
// 		goto end;

	 /* initialize the PS/2 controller */
	if(init_ps2_controller(xsdp) == 1)
		goto end;

	
	/* initialize the keyboard driver */
	if(init_keyboard_driver() == 1)
		goto end;

	uint8_t key;
	uint8_t last_key_pressed = 0x0;
	char key_char;

	while(1) {
		key = inportb(0x60);

		/* if( key != last_key_pressed || key == 0xfa || key == 0xab || key == 0xaa || key == 0x0)
		 *	continue;
		 */

		if(key == last_key_pressed)
			continue;


		else if(key == 0x13)
			key_char = 'r';

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
