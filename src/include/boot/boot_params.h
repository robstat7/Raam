#ifndef BOOT_PARAMS_H
#define BOOT_PARAMS_H

#include <boot/fb.h>
#include <boot/xsdp.h>

struct memory_map_struct {
	uint64_t memory_map_size;	/* memory map buffer size in bytes */
	uint64_t desc_size;
	char *memory_map_base;
};

struct stack_pmm_struct {
	const char *stack;
	uint64_t total_pages;
};

struct boot_params {
	struct fb_info_struct fb_info;
	const struct xsdp_struct *xsdp;	// pointer to xsdp structure
	const uint8_t *system_variables;
	struct memory_map_struct memory_map;
	struct stack_pmm_struct stack_pmm;
};

extern struct boot_params boot_params;

#endif	// BOOT_PARAMS_H
