/*
 * Raam Raam sa _/\_ _/\_ _/\_
 *
 * Boot loader written using gnu-efi
 */
#include <efi.h>
#include <efilib.h>

EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
EFI_STATUS return_status;
UINTN num_modes, native_mode;

EFI_STATUS store_framebuffer_info(void);
EFI_STATUS detect_gop(void);
EFI_STATUS set_1280_by_1024_video_mode(void);
EFI_STATUS get_current_video_mode(void);

/*
 * efi_main
 *
 * This function initializes the UEFI library and stores the framebuffer
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
 * This function detects the graphics output protocol (GOP), sets 1280 x 1024
 * video mode, and stores the framebuffer information to be passed in the boot
 * parameters to the kernel.
 */
EFI_STATUS store_framebuffer_info(void)
{
	return_status = detect_gop();
	if(EFI_ERROR(return_status))
		return return_status;

	return_status = set_1280_by_1024_video_mode();

	return return_status;
}

/*
 * set_1280_by_1024_video_mode
 *
 * This function gets the current mode, queries the available video modes, and
 * sets the 1280 x 1024 video mode for better text visibility.
 */
EFI_STATUS set_1280_by_1024_video_mode(void)
{
	return_status = get_current_video_mode();
	if(EFI_ERROR(return_status))
		return return_status;
	else
		Print(L"@got native mode!\n");
}

/*
 * get_current_video_mode
 *
 * This function gets the current video mode.
 */
EFI_STATUS get_current_video_mode(void)
{
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
	UINTN sizeof_info;

	return_status = uefi_call_wrapper(gop->QueryMode,4,gop,
					  gop->Mode==NULL?0:gop->Mode->Mode,
					  &sizeof_info, &info);
	// this is needed to get the current video mode
	if(return_status == EFI_NOT_STARTED)
		return_status = uefi_call_wrapper(gop->SetMode,2,gop,0);
	if(EFI_ERROR(return_status)) {
		Print(L"unable to get native mode\n");
	} else {
		native_mode = gop->Mode->Mode;
		num_modes = gop->Mode->MaxMode;
	}
	return return_status;
}

/*
 * detect_gop
 *
 * This function detects the GOP.
 */
EFI_STATUS detect_gop(void)
{
	EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

	/* invoke BootServices->LocateProtocol function */
	return_status = uefi_call_wrapper(BS->LocateProtocol,3,&gop_guid,NULL,
					  (void **) &gop);
	return return_status;
}
