/*
 * boot services related functions.
 */
#include <boot/boot.h>

UINTN memory_map_size = 0, map_key = 0, desc_size = 0;

/*
 * exit_boot_services
 *
 * This function gets the memory map and tries to exit boot services 3
 * times.
 */
EFI_STATUS exit_boot_services(void)
{
	EFI_STATUS status;

	status = get_memory_map();
	if(EFI_ERROR(status)) {
		goto end;
	}
end:
	return status;
}

/*
 * get_memory_map
 *
 * This function gets the memory map size, updates it, and allocates a new
 * pool of memory for the memory map.Then it gets the memory map into
 * this newly allocated pool.
 */
EFI_STATUS get_memory_map(void)
{
	EFI_STATUS status;

	status = get_memory_map_size();
	if(status != EFI_BUFFER_TOO_SMALL) {
		goto end;
	}
end:
	return status;
}

/*
 * get_memory_map_size
 *
 * This function gets the memory map size.
 */
EFI_STATUS get_memory_map_size(void)
{
	EFI_STATUS status;

	status = uefi_call_wrapper(BS->GetMemoryMap,5,&memory_map_size,0,
				   &map_key,&desc_size,NULL);	
	if(status != EFI_BUFFER_TOO_SMALL) {
		Print(L"fatal error: error getting memory map size!\n");
		memory_map_size = 0;
	}

	return status;
}
