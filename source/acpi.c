/*
 * Parsing ACPI tables for system information
 */
#include "string.h"
#include "printk.h"
#include <stdint.h>

unsigned char check_xsdt_checksum(uint64_t *xsdt, uint32_t xsdt_length);

uint64_t * get_madt_pointer(uint64_t *xsdp)
{

	uint64_t *madt = NULL;
	char desc_header_sig[4];

	/* get the physical address of XSDT structure */
	volatile uint64_t *xsdt = (uint64_t *) *((uint64_t *) ((char *) xsdp + 24));

	uint32_t xsdt_length = *((uint32_t *) ((unsigned char *) xsdt + 4));

	/* check for valid XSDT checksum */
	if(check_xsdt_checksum(xsdt, xsdt_length) != 0) {
		printk("error: invalid xsdt table!\n");
		return NULL;
	}

	int num_entries = (xsdt_length - 36)/8 ;

	/* find and store MADT table pointer */
	for(int i = 0; i < num_entries; i++) {
		uint64_t *desc_header = (uint64_t *) ((uint64_t *) ((char *) xsdt + 36))[i];

		strncpy(desc_header_sig, (char *) desc_header, 4);

		/* check MADT table signature */
		if(strncmp(desc_header_sig, "APIC", 4) == 0) {
			madt = desc_header;
			break;
		}
	}

	if(madt == NULL) {
		printk("error: could not find MADT table!\n");
		return madt;
	}

	printk("@acpi: found madt. addr = {p}\n", (void *)madt);

	return madt;
}

/*
 * returns 0 if the checksum is valid.
 */
unsigned char check_xsdt_checksum(uint64_t *xsdt, uint32_t xsdt_length)
{
    unsigned char sum;
    int i;

    sum = 0;

    for(i = 0; i < xsdt_length; i++) {
	sum += ((char *) xsdt)[i];
    }

    return sum;
}
