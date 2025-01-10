#include <boot/xsdp.h>

// EFI GUID for a pointer to the ACPI 2.0 or later specification XSDP structure
EFI_GUID Acpi20TableGuid = ACPI_20_TABLE_GUID;

void *xsdp = NULL;

int get_xsdp_pointer(EFI_SYSTEM_TABLE *system_table)
{
	int num_config_tables = system_table->NumberOfTableEntries;
	int ret = 0;

	EFI_CONFIGURATION_TABLE *config_tables = system_table->ConfigurationTable;

	for(int i = 0; i < num_config_tables; i++) {
                if(CompareGuid(&config_tables[i].VendorGuid, &Acpi20TableGuid) == 0) {
                        uint8_t *table = config_tables[i].VendorTable;
			// check if ACPI version is >= 2.0
			uint8_t revision = *(table + 15);	
			if(revision == 0x2) {
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
