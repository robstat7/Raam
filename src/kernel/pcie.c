#include <raam/pcie.h>
#include <raam/acpi.h>

struct ecam_struct ecam;

// get and store ECAM base address and starting and ending pcie bus numbers.
void pcie_init(void)
{
	// in most systems there is only one PCI segment group -
	// (PCI segment group number 0). Hence use 0 index.
	ecam.base = (uint64_t *) acpi_tables.mcfg->e[0].base_addr;
	ecam.start_bus_num = acpi_tables.mcfg->e[0].start_bus_num;
	ecam.end_bus_num = acpi_tables.mcfg->e[0].end_bus_num;

	printk("@ecam.base = {d}  ", ecam.base);
	printk("@start_bus_num = {d}  ", ecam.start_bus_num);
	printk("@end_bus_num = {d}  ", ecam.end_bus_num);
}
