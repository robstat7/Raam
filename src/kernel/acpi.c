#include <raam/acpi.h>

struct xsdt_struct *xsdt;
const int xsdt_array_size = 8;  // see definition of `xsdt_struct` in headerfile

int acpi_init(const struct xsdp_struct *xsdp)
{
	int ret = find_xsdt_table(xsdp);
	if(ret == -1) {
		goto end;
	}

	ret = get_mcfg_pointer();
end:
	return ret;
}

static int get_mcfg_pointer(void)
{
	int enteries = (xsdt->h.length - sizeof(xsdt->h)) / xsdt_array_size;
	int ret = 0;

	printk("@enteries = {d}  ", enteries);

	ret = find_valid_mcfg(enteries);

	return ret;
}

static int find_valid_mcfg(int enteries)
{
	int ret = 0;

	//debug
// 	uint64_t *expected_start = (uint64_t *)((char *)xsdt + sizeof(struct acpi_sdt_header));
// if (xsdt->pointer_to_other_sdts != expected_start) {
// 	printk("different addrs  ");
// }
// else {
// 	printk("same addrs  ");
// }
// 
// printk("differnce is {d}  ", xsdt->pointer_to_other_sdts - expected_start);
// for(;;);	

	for(int i = 0; i < enteries; i++) {
		const struct acpi_sdt_header *desc_header =
		      (struct acpi_sdt_header *) xsdt->pointer_to_other_sdts[i];
		// const struct acpi_sdt_header *desc_header = (struct acpi_sdt_header *) ((uint64_t *) ((char *) xsdt + 36))[i];
		/* check MCFG table signature */                                
                if(strncmp(desc_header->signature, "MCFG", 4) == 0) {
			printk("@found mcfg table!  ");
		}
	}

	return ret;
}

/*
 * find_xsdt_table
 *
 * This function parses the xsdt address from the xsdp structure and
 * then its length and validates its checksum. It returns 0 on a valid
 * xsdt table else -1.
 */
static int find_xsdt_table(const struct xsdp_struct *xsdp)
{
	xsdt = (struct xsdt_struct *) xsdp->xsdt_address;

	int ret = 0;

	printk("@xsdt->h.length = {d}  ", xsdt->h.length);

	// validate checksum
	if(validate_checksum((uint8_t *) xsdt, xsdt->h.length) != 0) {
		printk("error: invalid xsdt table!\n");
		ret = -1;
	}

	return ret;
}
