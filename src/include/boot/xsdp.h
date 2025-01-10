#ifndef XSDP_H
#define XSDP_H

#include <efi.h>
#include <efilib.h>
#include <stdint.h>

#define RSDP_START_OFFSET 0
#define RSDP_END_OFFSET 19
#define XSDP_START_OFFSET 20
#define XSDP_END_OFFSET 35

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
int find_valid_xsdp(UINTN num_tables, EFI_CONFIGURATION_TABLE *tables);
int validate_xsdp(struct xsdp_struct *table);
int check_valid_acpi_version(struct xsdp_struct *table);
int8_t validate_checksum(struct xsdp_struct *table);

#endif	// XSDP_H
