/*
 * Raam Raam sa _/\_ _/\_ _/\_
 *
 * Boot loader written using gnu-efi
 */
#include <boot/fb.h>
#include <boot/boot.h>

/*
 * efi_main
 *
 * This function initializes the UEFI library, stores the framebuffer
 * information, and exits the boot services.
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

	status = exit_boot_services(image_handle);
	if(EFI_ERROR(status)) {
		goto hang; 
	}

hang:
	for(;;);
end:
	return EFI_SUCCESS;
}
