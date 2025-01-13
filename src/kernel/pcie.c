#include <raam/pcie.h>
#include <raam/acpi.h>

void pcie_init(void)
{
	int enteries = (acpi_tables.mcfg->h.length - (sizeof(struct acpi_sdt_header) + sizeof(uint64_t))) / sizeof(struct enhanced_config_base_struct);

	printk("@enteries = {d}  ", enteries);

	for(int i = 0; i < enteries; i++) {
		if(acpi_tables.mcfg->e[i].pci_seg_grp_num == 0x0) {
			printk("@found pci seg grp num 0  ");
			break;
		}
	}
}
