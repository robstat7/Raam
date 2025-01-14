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
	uint16_t bus;                                                              
	uint8_t dev;	// device
	int found = 0;

	for(bus = pcie_ecam.start_bus_num; bus <= pcie_ecam.end_bus_num;
	    bus++) {
		for(dev = 0; dev < 32; dev++) {	// there can be up to 32 devices
						// on a bus
			if(check_device_for_controller(bus, dev) == 0) {
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

/*
 * check_device_for_controller
 * ---------------------------
 * This function checks all the devices' function 0 on a given bus to
 * find the controller.
 *
 * Parameters:
 *   bus  - The PCIe bus number (0-255)
 *   dev  - The device number on the specified bus (0-31)
 *   func - The function number within the device (0)
 *
 * Returns:
 *   0 if the controller is found else -1.
 *
 * Notes:
 *   - function 0 is probed because most NVMe controllers are
 *     single-function device.
 *   - `volatile` is used for MMIO access to ensure that the compiler
 *     won't optimize hardware reads (we won't read cached values).
 *   - The Class Code, Subclass, and Prog IF register values are used
 *     to identify the device's type, the device's function, and the
 *     device's register-level programming interface.
 */
static int check_device_for_controller(uint16_t bus, uint8_t dev)
{
	const uint8_t func = 0;		/* probe function 0 */
	uint16_t vendor_id;
	int ret = -1;

	volatile struct common_config_space_header_struct *h =
				(struct common_config_space_header_struct *)
				get_config_space_phy_mmio_addr(bus, dev, func);
	
	/* check if a device is present */
	if (h->vendor_id != PCI_INVALID_VENDOR_ID) {
		/* check for nvme class, subclass, and programming interface */
		if(h->class_code == NVME_CLASS_CODE &&
		   h->subclass == NVME_SUBCLASS &&
		   h->prog_if == NVME_PROG_IF) {
			/* found the controller! */
			ret = 0;
		}
	}
	return ret;
}

/*
 * get_config_space_phy_mmio_addr
 * ------------------------------
 * This function returns the physical MMIO address of the PCI
 * configuration space for the function.
 *
 * Parameters:
 *   bus  - The PCIe bus number (0-255)
 *   dev  - The device number on the specified bus (0-31)
 *   func - The function number within the device (0-7)
 *
 * Returns:
 *   The 64-bit unsigned integer number that is the physical MMIO
 *   address of the PCI configuration space for the function.
 *
 * Notes:
 *   - This function assumes the use of a PCIe ECAM (Enhanced
 *     Configuration Access Mechanism) to access PCI configuration
 *     space.
 *   - The PCI configuration space for each function is 4096 bytes in size.

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
