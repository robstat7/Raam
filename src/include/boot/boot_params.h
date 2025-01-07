#ifndef BOOT_PARAMS_H
#define BOOT_PARAMS_H

#include <boot/fb.h>

struct boot_params {
	struct fb_info	fb_info;
};

extern struct boot_params boot_params;

#endif	// BOOT_PARAMS_H
