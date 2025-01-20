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
#include <raam/main.h>

/*
 * efi_main
 *
 * This function initializes the UEFI library, stores the framebuffer
 * information, exits the boot services, and calls the kernel's main
 * function passing the boot parameters.
 *
 * TODO: incomplete docstring.
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

	status = exit_boot_services(image_handle);
	if(EFI_ERROR(status)) {
		goto hang; 
	}

	/* call the kernel's main function */
	main(boot_params);

hang:
	for(;;);
end:
	return EFI_SUCCESS;
}
