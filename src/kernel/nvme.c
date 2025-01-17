/*
 * NVMe over PCIe driver.
 *
 * specification used:
 *   NVM Express Revision 1.3
 */
#include <raam/nvme.h>
#include <raam/pcie.h>
#include <raam/printk.h>

volatile struct register_map_struct *register_map;
char *data_region_creation_addr;
char *nvme_asqb;	/* admin submission queue base addr 4K aligned */
char *nvme_acqb;	/* admin completion queue base addr 4K aligned */
char *controller_identify_prp_base; 	/* 4K controller identify PRP base
					   address */
char *nvme_admin_tail;

int nvme_init(const uint8_t *system_variables)
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

	printk("@system_variables = {p}  ", (void *) system_variables);

	data_region_creation_addr = (char *) system_variables;

	configure_admin_queues();

	/* enable the controller - set CC.EN (bit #0) to 1 */
        register_map->cc |= 0x1;
        
	printk("@new CC value={d} ", register_map->cc);            

	if(nvme_init_enable_wait() == false) {                                      
                printk("fatal error: nvme: CSTS.CFS (bit #1) is not 0!\n");         
                return -1;                                                       
        } 

	/* assign address for the nvme admin tail value store */
	data_region_creation_addr =
		     get_next_4096_alligned_address(data_region_creation_addr);

	nvme_admin_tail = data_region_creation_addr;

	/* get the identify controller data structure */
        get_identify_controller_data_structure();

end:
	return ret;
}

// TODO: add comments and refactor
static void nvme_admin_wait(uint32_t *acqb_ptr)                                         
{                                                                               
        uint32_t val;                                                           
                                                                                
        do{                                                                     
                val = *acqb_ptr;                                              
                                                                                
        }while(val == 0);                                                       
                                                                                
        printk("acq: status field plus phase tag value: {p}  ",
	       (void *) *(uint16_t *) ((char *) acqb_ptr + 2));
                                                                                
        *acqb_ptr = 0; /* overwrite the old entry */
}

// TODO: add comments and refactor
static void nvme_admin_savetail(const uint32_t admin_tail_val,
				char* nvme_admin_tail,
			 	uint32_t old_admin_tail_val)
{
	*nvme_admin_tail = admin_tail_val;

	register_map->sq0tdbl = admin_tail_val;

	/* check completion queue */
	old_admin_tail_val *= 16;	/* each entry is 16 bytes */
	old_admin_tail_val += 12;	/* add 12 for double word 3 */

	char *acqb_ptr = nvme_acqb + old_admin_tail_val;

	nvme_admin_wait((uint32_t *) acqb_ptr);
}

// TODO: add comments and refactor
/*
 * nvme_admin
 * ----------
 * This function performs an admin operation on the controller.
 */
static void nvme_admin(const uint32_t cdw0, const uint32_t cdw1,
		       const uint32_t cdw10, const uint32_t cdw11,
		       const char *cdw6_7)
{
	/* build the command at the expected location in the submission ring */

	/* get the current admin tail value. Valid values are 0 to 63. */
	int admin_tail_val = *nvme_admin_tail;
	/* multiply by 64 as we are using 64 entries */
	admin_tail_val *= 64;
	/* find the pointer in the submission ring */
	char *asqb_ptr = nvme_asqb + admin_tail_val;

	/* build the structure */
        *(uint32_t *) asqb_ptr = cdw0;	/* cdw0 */
        *(uint32_t *) (asqb_ptr + 4) = cdw1;	/* cdw1 */
        *(uint32_t *) (asqb_ptr + 8) = 0;	/* cdw2 */
        *(uint32_t *) (asqb_ptr + 12) = 0;	/* cdw3 */
        *(uint64_t *) (asqb_ptr + 16) = 0;	/* cdw4-5 */
        *(uint64_t *) (asqb_ptr + 24) = (uint64_t) cdw6_7;	/* cdw6-7 */
        *(uint64_t *) (asqb_ptr + 32) = 0;	/* cdw8-9 */
        *(uint32_t *) (asqb_ptr + 40) = cdw10;	/* cdw10 */
        *(uint32_t *) (asqb_ptr + 44) = cdw11;	/* cdw11 */
        *(uint32_t *) (asqb_ptr + 48) = 0;	/* cdw12 */
        *(uint32_t *) (asqb_ptr + 52) = 0;	/* cdw13 */
        *(uint32_t *) (asqb_ptr + 56) = 0;	/* cdw14 */
        *(uint32_t *) (asqb_ptr + 60) = 0;	/* cdw15 */

	/* start the admin command by updating the tail doorbell */

	/* get the current admin tail value */
	admin_tail_val = *nvme_admin_tail;
	/* save the old admin tail value for reading from the completion ring */
	uint32_t old_admin_tail_val = admin_tail_val;
	admin_tail_val++;

	if(admin_tail_val == 64) {
		admin_tail_val = 0; /* wrap after 64 commands */
	}

	nvme_admin_savetail(admin_tail_val, nvme_admin_tail,
			    old_admin_tail_val);
}

/*
 * get_identify_controller_data_structure
 * --------------------------------------
 * this function gets the identify controller data structure in a data
 * buffer. The data structure is 4096 bytes in size.
 *
 * notes:
 *   - command dword 0 (cdw0) consists of command identifier as 0,
 *     physical region page (PRP) is used for this transfer (bits 15:14
 *     clear), fused operation (FUSE) is a normal operation (bits 9:8
 *     clear), and the opcode (bits 7:0) of the command is 0x6
 *     (identify).
 *     The physical memory location in memory to use for data transfer
 *     is specified using PRP. 
 *   - command dword 10 (cdw10) is specific to identify command.
 *     Controller or Namespace Structure (CNS) (bits 7:0) should be 0x1
 *     to identify the controller data structure for the controller.
 *     Rest of the bits should be 0.
 *   - command dword 1 and command dword 11 are ignored (set to 0). Rest
 *     all the commands must be 0.
 */
static void get_identify_controller_data_structure(void)
{
	const uint32_t cdw0 = CMD_IDENTIFY;	/* upper bits are 0 */
	const uint32_t cdw1 = CMD_IGNORED;
	const uint32_t cdw10 = CNS_CONTROLLER;
	const uint32_t cdw11 = CMD_IGNORED;

	data_region_creation_addr =
		     get_next_4096_alligned_address(data_region_creation_addr);

	controller_identify_prp_base = data_region_creation_addr;

	printk("@controller_identify_prp_base region val before submitting "
	       "identify cmd = {p}  ",
	       (void *) (*(uint64_t *) controller_identify_prp_base));

	nvme_admin(cdw0, cdw1, cdw10, cdw11, controller_identify_prp_base);

	printk("@controller_identify_prp_base region val after submitting "
	       "identify cmd = {p}  ",
	       (void *) (*(uint64_t *) controller_identify_prp_base));
}

/* update address to point to the next 4096 bytes alligned address.
This function assumes that the address in the parameter is already 4096
bytes alligned. */
static char *get_next_4096_alligned_address(char *addr)
{
	addr += PAGE_SIZE;

	return addr;
}

static bool nvme_init_enable_wait(void)
{ 
	/* poll until CSTS.RDY (bit #0) becomes 1, or return false if CSTS.CFS
	   (bit #1) is set */
	while((register_map->csts & 0x1) != 0x1) {
		/* check if CSTS.CFS (bit #1) is set (fatal error) */
		if(register_map->csts & 0x2) {
			return false;
		}
	}

	return true;
}
                                                                               
/* 
 * align_to_4096
 * -------------
 * This function aligns a given address to the next 4KB boundary.
 */
char *align_to_4096(char *addr)
{
	uint64_t address = (uint64_t) addr;

	/* if the address is not aligned, the remainder tells us how far
	   the address is from the previous 4KB boundary. */
	if(address % 4096 != 0) {
		address += 4096 - (address % 4096);
	}

	return (char *) address;
}

/*
 * configure_admin_queues
 * ----------------------
 * Configures the admin submission and completion queues by setting the
 * admin queue attributes (AQA) first. The attributes include the admin
 * completion queue size (ACQS) and the admin submission queue size
 * (ASQS). Both are set to 64 commands/entries, which are 0-based values
 * (i.e., 63).
 * Then set up the admin submission queue base and completion queue base
 * addresses (asqb and acqb) and set admin submission queue (asq) and
 * admin completion queue (acq) registers of the controller.
 */
static void configure_admin_queues(void)
{
	/* define the 0-based size of admin queues (63 for 64 commands). */
	const uint32_t QUEUE_SIZE = 0x3f;
	
	/* combine ACQS (bits 27:16) and ASQS (bits 11:0) into the
	   AQA register value. */
	uint32_t aqa_value = (QUEUE_SIZE << 16) | QUEUE_SIZE;
	
	/* set the admin queue attributes register (AQA). */
	register_map->aqa = aqa_value;
	
	printk("admin queue attributes (aqa) register value: {p}  ",
	       (void *) register_map->aqa);

	// set up asq and acq memory regions, ensuring 4KB alignment
	nvme_asqb = align_to_4096(data_region_creation_addr);
	data_region_creation_addr = nvme_asqb + 4096;
	
	nvme_acqb = align_to_4096(data_region_creation_addr);
	data_region_creation_addr = nvme_acqb + 4096;
	
	printk("@new asqb={p}", (void *) nvme_asqb);
	printk("@new acqb={p}", (void *) nvme_acqb);
	
	// Set asq and acq registers in the controller (must be 4KB aligned)
	register_map->asq = (uint64_t) nvme_asqb;
	register_map->acq = (uint64_t) nvme_acqb;
	
	printk("@read asq register={p}", (void *) register_map->asq);
	printk("@read acq register={p}", (void *) register_map->acq);
}

/*                                                                              
 * wait_for_reset_complete                                                      
 *                                                                              
 * wait for the controller to indicate that the previous reset is complete by   
 * waiting for CSTS.RDY to become ‘0'.                                          
 */                                                                             
static bool wait_for_reset_complete(void)                                              
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

static bool reset_controller(void)                                                     
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
static uint64_t *get_nvme_base(struct nvme_pcie_dev_info_struct
			       *controller_info)
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
