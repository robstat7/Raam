#include <raam/pcie.h>
#include <raam/acpi.h>

// get and store ECAM base address and starting and ending pcie bus numbers.
void pcie_init(void)
{
	struct pcie_ecam_struct pcie_ecam;

	// in most systems there is only one PCI segment group -
	// (PCI segment group number 0). Hence use 0 index.
	pcie_ecam.base = (uint64_t *) acpi_tables.mcfg->e[0].base_addr;
	pcie_ecam.start_bus_num = acpi_tables.mcfg->e[0].start_bus_num;
	pcie_ecam.end_bus_num = acpi_tables.mcfg->e[0].end_bus_num;

	printk("@ecam.base = {p}  ", (void *) pcie_ecam.base);
	printk("@start_bus_num = {d}  ", pcie_ecam.start_bus_num);
	printk("@end_bus_num = {d}  ", pcie_ecam.end_bus_num);
}
