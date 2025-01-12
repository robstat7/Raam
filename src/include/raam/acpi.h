#ifndef ACPI_H
#define ACPI_H

#include <boot/xsdp.h>
#include <raam/printk.h>

struct acpi_sdt_header {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
};

struct xsdt_struct {
	struct acpi_sdt_header h;
};	

void acpi_init(const struct xsdp_struct *xsdp);
static void find_xsdt_table(const struct xsdp_struct *xsdp);

#endif	// ACPI_H
