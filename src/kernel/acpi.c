#include <raam/acpi.h>

struct xsdt_struct *xsdt;

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
	int enteries = (xsdt->h.length - sizeof(xsdt->h)) / 8;
	int ret = 0;

	printk("@entries = {d}", enteries);

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

	// validate checksum
	if(validate_checksum((uint8_t *) xsdt, xsdt->h.length) != 0) {
		printk("error: invalid xsdt table!\n");
		ret = -1;
	}

	return ret;
}
