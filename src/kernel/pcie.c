#include <raam/pcie.h>
#include <raam/acpi.h>

void pcie_init(void)
{
	printk("@pci_seg_grp_num = {d}  ", acpi_tables.mcfg->e[0].pci_seg_grp_num);	
}
