/*
 * boot services related functions.
 */
#include <boot/boot.h>
#include <boot/mem.h>

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
