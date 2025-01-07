/*
 * boot time framebuffer related functions.
 */
#include <boot/fb.h>

EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

/*
 * store_framebuffer_info
 *
 * This function detects the graphics output protocol (GOP) and stores the
 * framebuffer information.
 */
EFI_STATUS store_framebuffer_info(void)
{
	EFI_STATUS status;

	status = detect_gop();
	if(EFI_ERROR(status)) {
		goto end;
	}

	Print(L"@native mode = %03d\n", gop->Mode->Mode);
	for(;;);
end:
	return status;
}

/*
 * detect_gop
 *
 * This function detects the GOP.
 */
EFI_STATUS detect_gop(void)
{
	EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_STATUS status;

	/* invoke BootServices->LocateProtocol function */
	status = uefi_call_wrapper(BS->LocateProtocol,3,&gop_guid,NULL,
				   (void **) &gop);
	if(EFI_ERROR(status)) {
		gop = NULL;
	}

	return status;
}
