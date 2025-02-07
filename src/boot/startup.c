/*
 * startup.c contains the kernel's startup code.
 */
#include <boot/startup.h>
#include <raam/main.h>
#include <boot/gdt.h>
#include <boot/idt.h>
#include <raam/tty.h>
#include <mm/memory.h>

void startup(struct boot_params boot_params)
{
	/* initialize the gdt */
	init_gdt();
	
	/* initialize the idt */
	init_idt();

	tty_init(boot_params.fb_info);

	/* phyical memory manager initialization */	
	pmm_init(boot_params.memory_map, boot_params.free_stack);

	for(;;);

	/* call the kernel's main function */
	main(boot_params);
}
