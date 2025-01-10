#include <boot/xsdp.h>

// EFI GUID for a pointer to the ACPI 2.0 or later specification XSDP structure
EFI_GUID acpi_20_table_guid = ACPI_20_TABLE_GUID;

void *xsdp = NULL;

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

	if(xsdp) {
		Print(L"found xsdp strucutre pointer!\n");
	} else {
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

	ret = validate_checksum(table);
end:
	return ret;
}

// Returns 0 on success.
// For ACPI 1.0 (the first structure) you add up every byte in the
// structure and make sure the lowest byte of the result is equal to
// zero. For ACPI 2.0 and later you'd do exactly the same thing for the
// original (ACPI 1.0) part of the second structure, and then do it
// again for the fields that are part of the ACPI 2.0 extension.
int8_t validate_checksum(struct xsdp_struct *table)
{
	int8_t *bytes = (int8_t *)table;
	int sum = 0;
	uint8_t i;
	
	// sum RSDP region
	for (i = RSDP_START_OFFSET; i <= RSDP_END_OFFSET; i++) {
	    sum += bytes[i];
	}
	
	// If RSDP checksum is valid, include XSDP region
	if ((int8_t)sum == 0) {
	    for (i = XSDP_START_OFFSET; i <= XSDP_END_OFFSET; i++) {
	        sum += bytes[i];
	    }
	}
	
	return (int8_t)sum;
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
