/*
 * boot time framebuffer related functions.
 */
#include <boot/fb.h>
#include <boot/boot_params.h>

EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
struct boot_params boot_params;

/*
 * store_framebuffer_info
 *
 * This function detects the graphics output protocol (GOP) and stores the
 * framebuffer information. This information will be passed in the boot
 * parameters to the kernel. Our terminal driver's initialization code will need
 * this.
 */
EFI_STATUS store_framebuffer_info(void)
{
	EFI_STATUS status;

	status = detect_gop();
	if(EFI_ERROR(status)) {
		goto end;
	}

	boot_params.fb_info.fb_base = gop->Mode->FrameBufferBase;
	boot_params.fb_info.horizontal_resolution =
					gop->Mode->Info->HorizontalResolution;
	boot_params.fb_info.vertical_resolution =
					gop->Mode->Info->VerticalResolution;
	boot_params.fb_info.pixels_per_scanline =
					gop->Mode->Info->PixelsPerScanLine;
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
