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
	int ret = 0;	// function default return value
	struct xsdp_struct *table;	// pointer to an xsdp table

	// check if any table has the valid efi guid for the xsdp structure
	for(UINTN i = 0; i < num_config_tables; i++) {
		if(CompareGuid(&config_tables[i].VendorGuid,
			       &acpi_20_table_guid) == 0) {
			// found an xsdp table.
                        table = (struct xsdp_struct *)
				 config_tables[i].VendorTable;
			// It might be a valid xsdp. Perform validations.
			// validation 1: check if ACPI version is >= 2.0
			uint8_t revision = table->revision;	
			if(revision == 0x2) {
                        	/* store xsdp struct pointer */
				xsdp = (void *) table;
				break;
			}
		}
	}

	if(xsdp == NULL) {                                                      
		Print(L"error: could not find XSDP structure pointer!\n");
		ret = -1;
	}
	else {
		Print(L"found xsdp strucutre pointer!\n");
	}

	return ret;
}
