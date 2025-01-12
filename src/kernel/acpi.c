#include <raam/acpi.h>

void acpi_init(const struct xsdp_struct *xsdp)
{
	find_xsdt_table(xsdp);
}

static void find_xsdt_table(const struct xsdp_struct *xsdp)
{
	const struct xsdt_struct *xsdt = (struct xsdt_struct *)
						xsdp->xsdt_address;
	const uint32_t length = xsdt->h.length;

	printk("@xsdt->h.length = {d}", xsdt->h.length);
}
