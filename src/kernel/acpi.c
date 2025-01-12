#include <raam/acpi.h>

int acpi_init(const struct xsdp_struct *xsdp)
{
	int ret = find_xsdt_table(xsdp);
	if(ret == -1) {
		goto end;
	}
end:
	return ret;
}

static int find_xsdt_table(const struct xsdp_struct *xsdp)
{
	const struct xsdt_struct *xsdt = (struct xsdt_struct *)
						xsdp->xsdt_address;
	const uint32_t length = xsdt->h.length;

	int ret = 0;

	printk("@xsdt->h.length = {d}", xsdt->h.length);

	// validate checksum
	if(validate_checksum((uint8_t *) xsdt, length) != 0) {
		printk("error: invalid xsdt table!\n");
		ret = -1;
	}

	return ret;
}
