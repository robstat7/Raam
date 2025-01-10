#ifndef XSDP_H
#define XSDP_H

#include <efi.h>
#include <efilib.h>
#include <stdint.h>

struct xsdp_struct {
	char signature[8];
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;
	uint32_t rsdt_address;      // deprecated since version 2.0
	
	uint32_t length;
	uint64_t xsdt_address;
	uint8_t extendedchecksum;
	uint8_t reserved[3];
};

int get_xsdp_pointer(EFI_SYSTEM_TABLE *system_table);

#endif	// XSDP_H
