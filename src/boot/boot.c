/*
 * boot services related functions.
 */
#include <boot/boot.h>
#include <boot/mem.h>

/*
 * exit_boot_services
 *
 * This function gets the memory map and tries to exit the boot services
 * 3 times.
 */
EFI_STATUS exit_boot_services(EFI_HANDLE image_handle)
{
	EFI_STATUS status;
	const int max_attempts = 3;

	status = get_memory_map();
	if(EFI_ERROR(status)) {
		goto end;
	}

	/* exit boot services */
	for(int i = 0; i < max_attempts; i++) {
		status = uefi_call_wrapper(BS->ExitBootServices, 2,
					   image_handle, map_key);
		if(status == EFI_SUCCESS) {
			break;
		}
	}

	if(status == EFI_INVALID_PARAMETER) {
		Print(L"fatal error: exit boot services: map key is "
		      "incorrect!\n");
		goto end;
	}
end:
	return status;
}
