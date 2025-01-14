/*
 * NVMe over PCIe driver.
 */
#include <raam/nvme.h>
#include <raam/pcie.h>
#include <raam/printk.h>

struct controller_info_struct {
	int16_t detected_bus_num;
	int16_t detected_device_num;
	int16_t detected_function_num;
}controller_info;

void nvme_init(void)
{
	find_controller();
}

/*
 * find_controller
 *
 * This function finds the nvme controller on all the pcie buses.
 */
static void find_controller(void)
{
	check_all_buses();
}

static void check_all_buses(void)
{
	uint16_t bus;                                                              
	uint8_t device;                                                            
	int found = 0;	

	for(bus = pcie_ecam.start_bus_num; bus <= pcie_ecam.end_bus_num;
	    bus++) {
		for(device = 0; device < 32; device++) {
			check_device(bus, device);
		}
	}
}

static void check_device(uint16_t bus, uint8_t dev)
{
	uint8_t func = 0;
	uint16_t vendor_id;
	
	vendor_id = get_vendor_id(bus, dev, func);
	if (vendor_id == 0xFFFF)        /* device doesn't exist */
		printk("@device doesn't exist!  ");
	else
		printk("vendor_id = {p}  ", (void *) vendor_id);
}

static uint16_t get_vendor_id(uint32_t bus, uint32_t dev, uint32_t func)
{
	// determine where the (4096-byte) area for a function's PCI
	// configuration space is.
	uint64_t mmio_start_physical_addr = (uint64_t) pcie_ecam.base;
	uint64_t addr = mmio_start_physical_addr + (bus << 20 | dev << 15 | func << 12);
	volatile uint64_t *phy_addr = (uint64_t *) addr;

	uint16_t vendor_id = *(volatile uint16_t *) phy_addr;
	return vendor_id;
}
