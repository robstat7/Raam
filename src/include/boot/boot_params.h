#ifndef BOOT_PARAMS_H
#define BOOT_PARAMS_H

#include <boot/fb.h>
#include <boot/xsdp.h>

struct memory_map_struct {
	uint64_t memory_map_size;	/* memory map buffer size in bytes */
	uint64_t desc_size;
	char *memory_map_base;
};

/* free stack (physical memory manager) data structure attributes */
struct free_stack_struct {
	char *free_stack_base;
	uint64_t size;
};

struct boot_params {
	struct fb_info_struct fb_info;
	const struct xsdp_struct *xsdp;	// pointer to xsdp structure
	const uint8_t *system_variables;
	struct memory_map_struct memory_map;
	struct free_stack_struct free_stack;
};

extern struct boot_params boot_params;

#endif	// BOOT_PARAMS_H
