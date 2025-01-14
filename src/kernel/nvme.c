/*
 * NVMe over PCIe driver.
 */
#include <raam/nvme.h>
#include <raam/pcie.h>
#include <raam/printk.h>

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
	uint8_t dev;	// device
	int found = 0;

	for(bus = pcie_ecam.start_bus_num; bus <= pcie_ecam.end_bus_num;
	    bus++) {
		for(dev = 0; dev < 32; dev++) {	// there can be up to 32 devices
						// on a bus
			if(check_device(bus, dev) == 0) {
				printk("@found nvme controller: bus {d}, "
				       "dev {d}, func {d}  ", bus, dev, 0);
				found = 1;
				break;
			}
			if(found == 1) {
				break;
			}
		}
	}
}

// This function checks all the devices on a given bus to find the controller.
static int check_device(uint16_t bus, uint8_t dev)
{
	const uint8_t func = 0;	// function 0 is used to probe whether a device
				// is present at a given bus and device address
	uint16_t vendor_id;
	int ret = 0;
	
	vendor_id = get_vendor_id(bus, dev, func);
	if (vendor_id == 0xffff) {        /* device doesn't exist */
		ret = -1;
		goto end;
	} else {			// device exists.
		ret = search_for_controller(bus, dev, func);
	}
end:
	return ret;
}

/*
 * get_vendor_id
 *
 * This function reads the vendor id of a PCIe device given its bus,
 * device, and function numbers.
 *
 * Parameters:
 *   bus  - The PCIe bus number (0-255)
 *   dev  - The device number on the specified bus (0-31)
 *   func - The function number within the device (0-7)
 *
 * Returns:
 *   The 16-bit vendor ID of the specified PCIe function. If the vendor
 *   ID is 0xffff, the specified function does not exist.
 *
 * Notes:
 *   - This function assumes the use of a PCIe ECAM (Enhanced
 *     Configuration Access Mechanism) to access PCI configuration
 *     space.
 *   - The PCI configuration space for each function is 4096 bytes in size.
 *
 * Specification used:
 *   PCI Express® Base Specification Revision 5.0 (section 7.5.1.1.1
 *   Vendor ID Register (Offset 00h))
 */
static uint16_t get_vendor_id(uint32_t bus, uint32_t dev, uint32_t func)
{
	// cast the calculated address to a pointer to the vendor ID field.
	// note: the vendor id is located at offset 0x00 of the configuration
	// space.
	volatile struct common_config_space_header_struct *h =
				(struct common_config_space_header_struct *)
				get_config_space_phy_mmio_addr(bus, dev, func);

	// read the vendor id from the calculated address.
	uint16_t vendor_id = h->vendor_id;

	return vendor_id;
}

/*
 * get_config_space_phy_mmio_addr
 *
 * This function returns the physical MMIO address of the PCI
 * configuration space for the function.
 *
 * Notes:
 *   - The ECAM layout formula can be found at:
 *     https://wiki.osdev.org/PCI_Express#Enhanced_Configuration_Mechanism
 *   - The base address of the ECAM region is stored in `pcie_ecam.base`.
 */
static uint64_t get_config_space_phy_mmio_addr(uint32_t bus, uint32_t dev,
					   uint32_t func)
{
	uint64_t mmio_start_physical_addr = (uint64_t) pcie_ecam.base;

	// compute the address using the ECAM layout formula:
	uint64_t config_space_mmio_phy_addr = mmio_start_physical_addr +
					  (bus << 20 | dev << 15 | func << 12);

	return config_space_mmio_phy_addr;
}

static int search_for_controller(uint32_t bus, uint32_t dev, uint32_t func)
{
	char *start_phy_addr = (char *)
				get_config_space_phy_mmio_addr(bus, dev, func);
	volatile uint32_t *phy_addr = (uint32_t *) (start_phy_addr + 8);
	uint32_t val = *phy_addr;
	val = val >> 8;
	int ret;

	if(val == 0x00010802) {  // class code = 0x1, subclass code = 0x8,
				   // prog if = 0x2
		ret = 0;
	} else {
		ret = -1;
	}

	return ret;
}
