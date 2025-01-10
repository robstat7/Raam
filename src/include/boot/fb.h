#ifndef FB_H
#define FB_H

#include <efi.h>
#include <efilib.h>

// structure to hold the framebuffer information
struct fb_info_struct {
	unsigned long long fb_base;	// framebuffer base address

	// horizontal and vertical screen resolutions
	unsigned long horizontal_resolution;
	unsigned long vertical_resolution;

	unsigned long pixels_per_scanline;
};

EFI_STATUS store_framebuffer_info(void);
EFI_STATUS detect_gop(void);

#endif	// FB_H
