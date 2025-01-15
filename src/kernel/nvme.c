/*
 * NVMe over PCIe driver.
 */
#include <raam/nvme.h>
#include <raam/pcie.h>
#include <raam/printk.h>

volatile struct register_map_struct *register_map;

int nvme_init(void)
{
	struct nvme_pcie_dev_info_struct controller;
	controller.found = 0;		// found is false

	int ret = 0;

	find_controller(&controller);

	if(controller.found == 0) {
		printk("error: nvme: couldn't find the controller!\n");
		ret = -1;
		goto end;
	}

	/* get nvme base address */
	uint64_t *nvme_base = get_nvme_base(&controller);

	/* initialize the register map pointer */
	register_map = (struct register_map_struct *) nvme_base;

	if(reset_controller() == false) {
		printk("error: nvme: the controller had a fatal error!\n");
		ret = -1;
		goto end;
	}

	printk("nvme: reset completed!  ");

end:
	return ret;
}

/*                                                                              
 * wait_for_reset_complete                                                      
 *                                                                              
 * wait for the controller to indicate that the previous reset is complete by   
 * waiting for CSTS.RDY to become ‘0'.                                          
 */                                                                             
bool wait_for_reset_complete(void)                                              
{                                                                               
	/* poll until CSTS.RDY (bit #0) becomes 0, or return false if CSTS.CFS
	   (bit #1) is set */
	while (register_map->csts & 0x1) {
		/* check if CSTS.CFS (bit #1) is set (fatal error) */
		if (register_map->csts & 0x2) {  
			return false;                                                       
		}                                                                       
	}                                                                           
	                                                                            
	return true;                                                                
}                   

bool reset_controller(void)                                                     
{                                                                               
	printk("@old cc value = {d}  ", register_map->cc);                            
	                                                                            
	/* clear CC.EN (bit #0) if it is set */
	register_map->cc &= ~0x1;                                                   
	                                                                            
	printk("@new cc value = {d}  ", register_map->cc);                            
	                                                                            
	return wait_for_reset_complete();                                           
}

/*
 * get_nvme_base
 * -------------
 * this function gets the nvme base address that we will use to
 * initialize the controller. First we get the header type that
 * identifies the layout of the rest of the header beginning at byte
 * 0x10 of the common header. If it is a standard header, we will read
 * the bar0 address and check if the bits 2-1 of the register is 0x2 or
 * 0x0 that means the base register is either 64-bit or 32-bit wide
 * respectively. If its 64-bit wide, the upper 32-bit are stored in bar1
 * register. Lastely, we clear the lowest 4 bits of the final base
 * address as they are not part of the address; instead, they serve
 * other purposes.
 *
 * parameters:
 *   controller  - controller info pointer of type
 *                 struct nvme_pcie_dev_info_struct
 *
 * returns:
 *   nvme base address (a uint64 pointer)
 *
 * resources used:
 *    - https://wiki.osdev.org/PCI
 */
uint64_t *get_nvme_base(struct nvme_pcie_dev_info_struct *controller_info)
{
	uint64_t base_addr;

	volatile struct common_config_space_header_struct *h =
			(struct common_config_space_header_struct *)
			get_config_space_phy_mmio_addr(controller_info->bus,
						       controller_info->dev,
						       controller_info->func);

	if(h->header_type == STANDARD_HEADER) {
		volatile struct header_type_0_table_struct *h0_table =
					(struct header_type_0_table_struct *) h;
		if((h0_table->bar0 & PCI_BAR_TYPE_MASK) == 0x4) { // Type is 0x2
			// i.e. base register is 64-bit wide
			base_addr = h0_table->bar1; // get upper 32 bits first
			base_addr <<= 32;	// left shift them
			// now store lower 32 bits of the base address
			base_addr |= h0_table->bar0;
		} else if((h0_table->bar0 & PCI_BAR_TYPE_MASK) == 0x0) {
			// Type is 0
			// i.e. base register is 32-bit wide
			base_addr = h0_table->bar0;	// obvious
		}

		base_addr &= ~0xf;  /* clear the lowest 4 bits */
		uint64_t *nvme_base = (uint64_t *) base_addr;

		return nvme_base;
	}
}

/*
 * find_controller
 * ---------------
 * This function finds the nvme controller on all the pcie buses.
 *
 * Parameters:
 *   nvme  - pointer to nvme device struct to store nvme info 
 *
 * Returns:
 *   void
 * 
 * Note:
 *   - only function 0 is probed because most NVMe controllers are
 *     single-function device.
 */
static void find_controller(struct nvme_pcie_dev_info_struct *controller)
{
	const uint8_t func = 0;		/* probe function 0 */

	for(uint16_t bus = pcie_ecam.start_bus_num;
	    bus <= pcie_ecam.end_bus_num; bus++) {
		for(uint8_t dev = 0; dev < MAX_PCI_BUS_DEV; dev++) {
			if(check_function(bus, dev, func) == 0) {
				/* found the controller. Store details. */
				controller->bus = bus;
				controller->dev = dev;
				controller->func = func;
				controller->found = 1; // found is true
				break;
			}
			if(controller->found == 1) {
				break;
			}
		}
	}
}

/*
 * check_function
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
 *   - `volatile` is used for MMIO access to ensure that the compiler
 *     won't optimize hardware reads (we won't read cached values).
 *   - The Class Code, Subclass, and Prog IF register values are used
 *     to identify the device's type, the device's function, and the
 *     device's register-level programming interface.
 */
static int check_function(uint16_t bus, uint8_t dev, uint8_t func)
{
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
