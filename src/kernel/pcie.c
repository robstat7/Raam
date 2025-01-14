#include <raam/pcie.h>
#include <raam/acpi.h>

struct pcie_ecam_struct pcie_ecam;

// get and store ECAM base address and starting and ending pcie bus numbers.
void pcie_init(void)
{
	// in most systems there is only one PCI segment group -
	// (PCI segment group number 0). Hence use 0 index.
	pcie_ecam.base = (uint64_t *) acpi_tables.mcfg->e[0].base_addr;
	pcie_ecam.start_bus_num = acpi_tables.mcfg->e[0].start_bus_num;
	pcie_ecam.end_bus_num = acpi_tables.mcfg->e[0].end_bus_num;
}
