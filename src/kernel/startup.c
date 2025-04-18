/*
 * startup.c contains the kernel's startup code.
 */
#include <raam/startup.h>
#include <raam/main.h>
#include <raam/gdt.h>
#include <raam/idt.h>
#include <raam/tty.h>
#include <raam/mm/memory.h>

void startup(struct boot_params boot_params)
{
	/* initialize the gdt */
	init_gdt();
	
	/* initialize the idt */
	init_idt();

	tty_init(boot_params.fb_info);

	/* phyical memory manager initialization */	
	pmm_init(boot_params.memory_map, boot_params.free_stack);

	/* call the kernel's main function */
	main(boot_params);
}
