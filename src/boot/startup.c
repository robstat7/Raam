/*
 * startup.c contains the kernel's startup code.
 */
#include <boot/startup.h>
#include <raam/main.h>
#include <boot/gdt.h>
#include <boot/idt.h>

void startup(struct boot_params boot_params)
{
	/* initialize the gdt */
	init_gdt();
	
	/* initialize the idt */
	init_idt();

	/* phyical memory manager initialization */	
	// pmm_init(boot_params.memory_map);

	/* call the kernel's main function */
	main(boot_params);
}
