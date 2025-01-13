#ifndef ACPI_H
#define ACPI_H

#include <boot/xsdp.h>
#include <raam/printk.h>
#include <lib/checksum.h>
#include <lib/string.h>
#include <raam/pcie.h>

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
}__attribute__((packed));;

// Note that the 'pointer_to_other_sdts' field as defined below should
// be aligned to a 4-byte boundary and not the default 8-byte alignment
// for a uint64_t. Source: XSDT - osdev wiki.
struct xsdt_struct {
	struct acpi_sdt_header h;

	// an array of 64-bit physical addresses that point to other
	// DESCRIPTION_HEADERs
	uint64_t pointer_to_other_sdts[] __attribute__((aligned(4)));
						      // force 4-byte alignment
}__attribute__((packed));	// force structure to be laid out with no gaps

struct mcfg_struct {
	struct acpi_sdt_header h;
	uint64_t reserved;
	struct enhanced_config_base_struct e[];
}__attribute__((packed));;

struct acpi_tables_struct {
	struct mcfg_struct *mcfg;
}__attribute__((packed));;

extern struct acpi_tables_struct acpi_tables;

int acpi_init(const struct xsdp_struct *xsdp);
static int find_xsdt_table(const struct xsdp_struct *xsdp);
static int get_mcfg_pointer(void);
static int find_valid_mcfg(int enteries);

#endif	// ACPI_H
