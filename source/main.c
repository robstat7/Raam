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


	put_red_pixel();

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

	
	// /* initialize the keyboard driver */
	// if(init_keyboard_driver() == 1)
	// 	goto end;

	// uint8_t key;

	// while(1) {
	// 	key = inportb(0x60);

	// 	if(key == 0xfa || key == 0xab)
	// 		continue;

	// 	printk("@key: {p}\n", (void *) key);
	// }

	/* print the command prompt */
	// printk("# _");

end:
	/* hang here */
	for(;;);

	return 1;
}
