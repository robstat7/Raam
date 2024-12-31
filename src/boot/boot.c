/*
 * Raam Raam sa _/\_ _/\_ _/\_
 *
 * Boot loader written using gnu-efi
 */
#include <efi.h>
#include <efilib.h>

EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
EFI_STATUS return_status;

EFI_STATUS store_framebuffer_info(void);
EFI_STATUS detect_gop(void);

/*
 * efi_main
 *
 * This routine initializes the UEFI library and stores the framebuffer
 * information.
 */
EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table)
{
	InitializeLib(image_handle, system_table);

	return_status = store_framebuffer_info();

	for(;;);

	return EFI_SUCCESS;
}

/*
 * store_framebuffer_info
 *
 * This routine detects the graphics output protocol (GOP), sets 1280 x 1024
 * video mode, and stores the framebuffer information to be passed in the boot
 * parameters to the kernel.
 */
EFI_STATUS store_framebuffer_info(void)
{
	return_status = detect_gop();
	if(EFI_ERROR(return_status))
		return return_status;

	Print(L"@located gop!\n");

	return return_status;
}

/*
 * detect_gop
 *
 * This routine detects the GOP.
 */
EFI_STATUS detect_gop(void)
{
	/* invoke BootServices->LocateProtocol function */
	return_status = uefi_call_wrapper(BS->LocateProtocol,
					      3,
					      &gop_guid,
					      NULL,
					      (void **) &gop);
	return return_status;
}
