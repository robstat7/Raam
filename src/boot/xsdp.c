#include <boot/xsdp.h>
#include <boot/boot_params.h>

// EFI GUID for a pointer to the ACPI 2.0 or later specification XSDP structure
EFI_GUID acpi_20_table_guid = ACPI_20_TABLE_GUID;

const void *xsdp = NULL;

/*
 * get_xsdp_pointer
 *
 * This function gets the xsdp structure pointer by examining the EFI
 * Configuration Table within the EFI System Table. Returns 0 on success
 * else -1.
 * Note: The boot loader must retrieve the pointer to the xsdp
 * structure before assuming platform control via the EFI
 * ExitBootServices interface.
 */
int get_xsdp_pointer(EFI_SYSTEM_TABLE *system_table)
{
	UINTN num_config_tables = system_table->NumberOfTableEntries;
	EFI_CONFIGURATION_TABLE *config_tables =
					system_table->ConfigurationTable;

	int ret = find_valid_xsdp(num_config_tables, config_tables); 	

	if(ret == 0) {	// success
		// store xsdp pointer in the boot params to be passed
		// to the kernel
		boot_params.xsdp = xsdp;
	}

	return ret;
}

// check if any table has the valid efi guid for the xsdp
// structure. Returns 0 on success else -1.
int find_valid_xsdp(UINTN num_tables, EFI_CONFIGURATION_TABLE *config_tables)
{
	int ret = 0;

	for(UINTN i = 0; i < num_tables; i++) {
		if(CompareGuid(&config_tables[i].VendorGuid, &acpi_20_table_guid) == 0) {
			struct xsdp_struct *table = (struct xsdp_struct *)
						   config_tables[i].VendorTable;
			// found an xsdp table. Now validate it.
			int success = validate_xsdp(table);
			if(success == 0) {
				xsdp = table;	
				break;
			}
		}
	}

	if(xsdp == NULL) {
		Print(L"error: could not find XSDP structure pointer!\n");
		ret = -1;
	}
	
	return ret;
}

// validates the xsdp table. Returns 0 on success else -1.
int validate_xsdp(struct xsdp_struct *table)
{
	int ret = check_valid_acpi_version(table);
	if(ret == -1) {
		goto end;
	}

	uint32_t length = table->length;
	ret = validate_checksum((uint8_t *) table, length);
end:
	return ret;
}

/*
 * validate_checksum
 *
 * This function sums all the bytes of the table and returns the lowest
 * byte of the sum. It returns 0 on success.
 */
uint8_t validate_checksum(const uint8_t *table, uint32_t length)
{
	uint32_t sum = 0;

	for(uint32_t i = 0; i < length; i++) {
		sum += table[i];
	}

	return (uint8_t) sum;
}

// check if ACPI version is >= 2.0. Returns 0 on success else -1.
int check_valid_acpi_version(struct xsdp_struct *table)
{
	if(table->revision == 2) {
		return 0;
	} else {
		return -1;
	}
}
