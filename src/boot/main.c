/*
 * Raam Raam sa _/\_ _/\_ _/\_
 *
 * Boot loader written using gnu-efi
 */
#include <boot/fb.h>
#include <boot/boot.h>
#include <boot/xsdp.h>
#include <boot/mem.h>
#include <boot/boot_params.h>
#include <boot/startup.h>
#include <asm/system.h>

/*
 * efi_main
 *
 * This function initializes the UEFI library, stores the framebuffer
 * information, exits the boot services, and calls the kernel's main
 * function passing the boot parameters.
 */
EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table)
{
	EFI_STATUS status;

	InitializeLib(image_handle, system_table);

	status = store_framebuffer_info();
	if(EFI_ERROR(status)) {
		goto end;
	}

	// get the xsdp pointer
	int ret = get_xsdp_pointer(system_table);
	if(ret == -1) {
		goto hang;
	}

	/* allocate and clear 2 MiB of memory for system variables */
        if(allocate_sys_variables_mem() == -1) {                     
                goto hang;
        }    

	Print(L"@boot/main.c: calling func to find total usable RAM pages\n");

	/* find free stack's (physical memory manager data structure) */
	/* maximum size. */
	const uint64_t free_stack_size =
		find_free_stack_size();
	if(free_stack_size == 0) {	/* error */
		goto hang;
	}

	/* allocate pool for free stack */
	char *free_stack_base = NULL;
	status = allocate_pool_for_free_stack(free_stack_size,
					      &free_stack_base);
	if(EFI_ERROR(status)) {                                                 
                goto hang;                                                       
        }

	Print(L"@free_stack_base = %p\n", (void *) free_stack_base);

	/* store free stack base and size in the boot params */
	boot_params.free_stack.free_stack_base = free_stack_base;
	boot_params.free_stack.size = free_stack_size;
	
	/* free the previous pool for mem map before getting the */
	/* new memory map for exiting boot services. */
	status = free_pool_for_mem_map();                                   
        if(EFI_ERROR(status)) {                                                 
                goto hang;                                                       
        }
	
	Print(L"@boot/main.c: calling func to exit BS\n");

	status = exit_boot_services(image_handle);
	if(EFI_ERROR(status)) {
		goto hang; 
	}

	/* disable interrupts */
	cli();

	/* call the kernel's startup function */
	startup(boot_params);

hang:
	for(;;);
end:
	return EFI_SUCCESS;
}
