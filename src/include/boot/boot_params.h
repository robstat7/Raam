#ifndef BOOT_PARAMS_H
#define BOOT_PARAMS_H

#include <boot/fb.h>
#include <boot/xsdp.h>

struct boot_params {
	struct fb_info_struct fb_info;
	const struct xsdp_struct *xsdp;	// pointer to xsdp structure
	const uint8_t *system_variables;
};

extern struct boot_params boot_params;

#endif	// BOOT_PARAMS_H
