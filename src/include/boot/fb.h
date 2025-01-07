#ifndef FB_H
#define FB_H

#include <efi.h>
#include <efilib.h>

EFI_STATUS store_framebuffer_info(void);
EFI_STATUS detect_gop(void);

#endif	// FB_H
