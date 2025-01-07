#ifndef FB_H
#define FB_H

#include <efi.h>
#include <efilib.h>

struct fb_info {
	unsigned long long fb_base;
	unsigned long horizontal_resolution;
	unsigned long vertical_resolution;
	unsigned long pixels_per_scan_line;
};

EFI_STATUS store_framebuffer_info(void);
EFI_STATUS detect_gop(void);

#endif	// FB_H
