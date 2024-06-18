/*
 * NVMe PCIe driver.
 */
#include "string.h"
#include "printk.h"
#include <stdint.h>

uint64_t *pcie_ecam = NULL;
int16_t detected_bus_num = -1;
int16_t detected_device_num = -1;
int16_t detected_function_num = -1;
volatile uint64_t *nvme_base = NULL;
char *nvme_data_region;
char *data_region_creation_addr;
char *nvme_asqb;
char *nvme_acqb;
char nvme_cc = 0x14;	// 4-byte Controller Configuration (CC) register

unsigned char check_xsdt_checksum(uint64_t *xsdt, uint32_t xsdt_length);
uint32_t check_mcfg_checksum(uint64_t *mcfg);
void check_all_buses(uint16_t start, uint16_t end);
int find_nvme_controller(uint16_t bus, uint8_t device, uint8_t function);
volatile uint64_t *get_nvme_base(uint16_t bus, uint8_t device, uint8_t function);
int reset_controller(void);
int configure_admin_q(void);
int wait_for_reset_complete(void);
void set_admin_q_attrs(void);
char *get_next_4096_aligned_addr(void);
void enable_controller(void);

int nvme_init(void *xsdp, char *sys_var_ptr)
{
	int i;
	uint64_t *xsdt;
	uint32_t xsdt_length;
	int num_entries;
	uint64_t *desc_header;
	uint64_t *mcfg;
	char desc_header_sig[4];
	uint16_t start_bus_num;
	uint16_t end_bus_num;

	mcfg = NULL;

	/* get physical address of the XSDT */
	xsdt = (uint64_t *) *((uint64_t *) ((char *) xsdp + 24));

	xsdt_length = *((uint32_t *) ((unsigned char *) xsdt + 4));

	/* check for valid XSDT checksum */
	if(check_xsdt_checksum(xsdt, xsdt_length) != 0) {
		printk("error: invalid xsdt table!\n");
		return 1;
	}

	num_entries = (xsdt_length - 36)/8 ;

	/* find and store MCFG table pointer */
	for(i = 0; i < num_entries; i++) {
		desc_header = (uint64_t *) ((uint64_t *) ((char *) xsdt + 36))[i];

		strncpy(desc_header_sig, (char *) desc_header, 4);

		/* check MCFG table signature */
		if(strncmp(desc_header_sig, "MCFG", 4) == 0) {
			/* check for valid MCFG checksum */
			if(check_mcfg_checksum(desc_header) == 0) {
				/* This is our MCFG table. Store its pointer */
				mcfg = desc_header;
				break;
			}
		}
	}

	if(mcfg == NULL) {
		printk("error: could not find MCFG table!\n");
		return 1;
	}

	/* get and store ECAM base address and starting and ending pcie bus number */
	pcie_ecam = (uint64_t *) *((uint64_t *) ((char *) mcfg + 44));
	start_bus_num = (uint16_t) *((unsigned char *) mcfg + 44 + 10);
	end_bus_num = (uint16_t) *(((unsigned char *) mcfg) + 44 + 11);

	/* enumerate pcie buses to find the nvme controller */
	check_all_buses(start_bus_num, end_bus_num);

	if(detected_bus_num != -1 && detected_device_num != -1 && detected_function_num != -1) {
		printk("found nvme controller! bus num={d}, device num={d}, function num={d}\n\n", detected_bus_num, detected_device_num, detected_function_num);
	} else {
		printk("couldn't found the nvme controller!\n");
		return 1;
	}

	/* get nvme base address */
	nvme_base = get_nvme_base(detected_bus_num, detected_device_num, detected_function_num);

	printk("@nvme_base={p}\n", (void *) nvme_base);

	/* reset the controller */
	if(reset_controller() == 1) {
		printk("nvme: error: the controller has had a fatal error!\n");
		return 1;
	}

	printk("@nvme: reset complete!\n");

	nvme_data_region = sys_var_ptr;

	data_region_creation_addr = nvme_data_region;

	/* configure the admin queue */
	configure_admin_q();

	/* enable controller */
	enable_controller();

	printk("@nvme: controller is enabled!\n");

	return 0;
}

/* 
 * set_admin_q_attrs
 *
 * set the Admin Queue Attributes (AQA) register for ACQS and ASQS values
 */
void set_admin_q_attrs(void)
{
	char nvme_aqa = 0x24;		// 4-byte AQA register offset
	volatile uint32_t *addr = (volatile uint32_t *) ((char *) nvme_base + nvme_aqa);
	uint32_t value = 0x003f003f;	// 64 commands each for ACQS (27:16) and ASQS (11:00).
					// Both are 0’s based values i.e. the value is 63.

	*addr = value;

	printk("@aqa register value={p}\n", (void *) *addr);
}

char *get_next_4096_aligned_addr(void)
{
	char *new_addr = data_region_creation_addr;

	while((((uint64_t) new_addr) % 4096) != 0) {
		new_addr++;
	}

	data_region_creation_addr = new_addr;

	return new_addr;
}

/*
 * configure_admin_q
 *
 * configure the admin queue
 */
int configure_admin_q(void)
{
	char nvme_asq = 0x28;   // 8-byte admin submission queue base address
	char nvme_acq = 0x30;	// 8-byte admin completion queue base address
	volatile uint64_t *addr_asq = (volatile uint32_t *) ((char *) nvme_base + nvme_asq);
	volatile uint64_t *addr_acq = (volatile uint32_t *) ((char *) nvme_base + nvme_acq);

	/* set the Admin Queue Attributes (AQA) register for ACQS and ASQS values */
	set_admin_q_attrs();

	/* get the next 4096 aligned address in the data region to be assigned as asqb */
	nvme_asqb = get_next_4096_aligned_addr();

	/* set data region creation address to the next 4096 aligned address */
	data_region_creation_addr = nvme_asqb + 4096;

	nvme_acqb = data_region_creation_addr;

	printk("@new asqb={p}\n", (void *) nvme_asqb);
	printk("@new acqb={p}\n", (void *) nvme_acqb);

	*addr_asq = (uint64_t) nvme_asqb;	// ASQB 4K aligned (63:12)
	*addr_acq = (uint64_t) nvme_acqb;	// ACQB 4K aligned (63:12)

	printk("@read asq register={p}\n", (void *) *addr_asq);
	printk("@read acq register={p}\n", (void *) *addr_acq);

	return 0;
}

/*
 * wait_for_reset_complete
 *
 * wait for the controller to indicate that the previous reset is complete by
 * waiting for CSTS.RDY to become ‘0'.
 */
int wait_for_reset_complete(void)
{
	char nvme_csts =  0x1C;			// 4-byte Controller Status (CSTS) register
	volatile uint32_t *addr = (volatile uint32_t *) ((char *) nvme_base + nvme_csts);
	uint32_t val;

	do {
		val = *addr;

		if((val & 0x2) != 0x0) {		// CSTS.CFS (bit #1) should be 0. If not the controller has had a fatal error
			return 1;
		}
	} while((val & 0x1) != 0x0);	// Wait for CSTS.RDY (bit #0) to become 0

	return 0;
}

void enable_controller(void)
{
	volatile uint32_t *addr = (volatile uint32_t *) ((char *) nvme_base + nvme_cc);
	uint32_t value;

	value = *addr;

	printk("@old CC value={d}\n", value);

	// set CC.EN (bit #0) to 1
	value |= 0x1;

	*addr = value;

	printk("@new CC value={d}\n", *addr);
}

int reset_controller(void)
{
	volatile uint32_t *addr = (volatile uint32_t *) ((char *) nvme_base + nvme_cc);
	uint32_t value;

	value = *addr;

	printk("@old CC value={d}\n", value);

	if((value & 0x1) != 0x0) {		// clear CC.EN (bit #0) to 0
		value &= 0xfffffffe;
		*addr = value;
	}

	printk("@new CC value={d}\n", *addr);

	/* wait for the reset to complete */
	return wait_for_reset_complete();
}

volatile uint64_t *get_nvme_base(uint16_t bus, uint8_t device, uint8_t function) {
	volatile char *phy_addr;
	uint32_t bar0_value, bar1_value;
	uint64_t base_addr;

	phy_addr = (volatile char *) ((uint64_t) pcie_ecam + (((uint32_t) bus) << 20 | ((uint32_t) device) << 15 | ((uint32_t) function) << 12));

	phy_addr = phy_addr + (4 * 4);	/* Multiplying by 4 because each register consists of 4 bytes */

	/* read register 4 for BAR0 */
	bar0_value = *(volatile uint32_t *) phy_addr;

	if((bar0_value & 0x6) == 0x4) {	// type (2:1) is 0x2 means the base register is 64-bits wide
		phy_addr += 4;	/* BAR1 is the register no. 5 */
		bar1_value = *(volatile uint32_t *) phy_addr;

		base_addr = bar1_value;
		base_addr <<= 32;		/* left shift 32 bits */
		base_addr |= (uint64_t) bar0_value;
	} else if((bar0_value & 0x6) == 0x0) {	/* type (2:1) is 0x0 means the base register is 32-bits wide */
		base_addr = bar0_value;
	}

	base_addr &= 0xfffffffffffffff0;		/* clear the lowest 4 bits */

	return (volatile uint64_t *) base_addr;
}


int find_nvme_controller(uint16_t bus, uint8_t device, uint8_t function) {
	uint64_t *phy_addr;
	uint32_t value;

	phy_addr = (uint64_t *) ((uint64_t) pcie_ecam + (((uint32_t) bus) << 20 | ((uint32_t) device) << 15 | ((uint32_t) function) << 12));

	phy_addr = (uint64_t *) ((char *) phy_addr + 8);

	value = *(volatile uint32_t *) phy_addr;
	
	value = value >> 8;

	if(value == 0x00010802) /* class code = 0x1, subclass code = 0x8, prog if = 0x2 */
		return 0;
	else
		return 1;
}

uint16_t get_vendor_id(uint16_t bus, uint8_t device, uint8_t function)
{
	uint64_t *phy_addr;
	uint16_t vendor_id;

	phy_addr = (uint64_t *) ((uint64_t) pcie_ecam + (((uint32_t) bus) << 20 | ((uint32_t) device) << 15 | ((uint32_t) function) << 12));

	vendor_id = *(volatile uint16_t *) phy_addr;

	return vendor_id;
}

int check_device(uint16_t bus, uint8_t device)
{
	uint8_t function = 0;
	uint16_t vendor_id;
	int res;

	vendor_id = get_vendor_id(bus, device, function);
	if (vendor_id == 0xFFFF)	/* device doesn't exist */
		return 1;

	res = find_nvme_controller(bus, device, function);	

	return res;
}

void check_all_buses(uint16_t start, uint16_t end)
{
     uint16_t bus;
     uint8_t device;	
     uint8_t found;

     found = 0;

     for(bus = start; bus <= end; bus++) {
         for(device = 0; device < 32; device++) {
		if(check_device(bus, device) == 0) {
			detected_bus_num = (int16_t) bus;
		 	detected_device_num = (int16_t) device;
			detected_function_num = 0;

			found = 1;
			break;
	 	}
        }
	if(found == 1) {
		 break;
	}
     }
}

/*
 * returns 0 if the checksum is valid.
 */
uint32_t check_mcfg_checksum(uint64_t *mcfg)
{
	uint32_t length;
	uint32_t sum;
	uint32_t i;

	sum = 0;

	length = *((uint32_t *) ((unsigned char *) mcfg + 4));

	for(i = 0; i < length; i++)
		sum += ((unsigned char *) mcfg)[i];

	return (sum & 0xff);
}

/*
 * returns 0 if the checksum is valid.
 */
unsigned char check_xsdt_checksum(uint64_t *xsdt, uint32_t xsdt_length)
{
    unsigned char sum;
    int i;

    sum = 0;

    for(i = 0; i < xsdt_length; i++) {
	sum += ((char *) xsdt)[i];
    }

    return sum;
}
