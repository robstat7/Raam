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

	/* find total usable main memory 4 KiB pages */
	const uint64_t total_usable_pages =
		find_num_usable_main_memory_4kib_pages();
	if(total_usable_pages == -1) {	/* error getting memory map */
		goto hang;
	}

	/* allocate pool for stack (physical memory manager data structure) */
	char *stack_pmm = NULL;
	status = allocate_pool_for_stack_pmm(total_usable_pages, &stack_pmm);
	if(EFI_ERROR(status)) {                                                 
                goto hang;                                                       
        }

	Print(L"@stack_pmm = %p\n", (void *) stack_pmm);

	/* store stack data structure base and total pages in the boot params */
	boot_params.stack_pmm.stack = stack_pmm;
	boot_params.stack_pmm.total_pages = total_usable_pages;

	// /* free the previous pool for mem map before getting the */
	// /* new memory map for exiting the boot services. */
	// status = free_pool_for_mem_map();                                   
        // if(EFI_ERROR(status)) {                                                 
        //         goto hang;                                                       
        // }

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
