/*
 * Raam Raam sa _/\_ _/\_ _/\_
 *
 * Kernel's main function
 */
#include <efi.h>

int main(void *xsdp, uint8_t *sys_var_ptr)
{
	/* initialize the global memory manager */
	// init_global_mm();	


//	__asm__ volatile ("mov $60, %eax; mov $0, %edi; syscall "); 
	

	// int b = 5/0;

	// int * a = NULL;
	// 
	// *a = 5;

	

	/* initialize the timer */
	// if(timer_init() == 1)
	// 	goto end;
	
	/* enable APIC interrupt controller */
	// enable_apic();


	// /* call test timer function */
	// test_timer();
	
	
	/* init xhci controller */
	if(xhci_init(xsdp) == 1)
		goto end;


	// /* init nvme */
	// if(nvme_init(xsdp, sys_var_ptr) == 1)
	// 	goto end;

end:
	/* hang here */
	for(;;);

	return 1;
}
