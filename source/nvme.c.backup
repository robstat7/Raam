/*
 * NVMe PCIe driver.
 */
#include <stdio.h>
#include "string.h"
#include "printk.h"
#include <stdint.h>

#define NVMe_VS		0x8	/* BAR0 Version register's offset */
#define NVMe_ANS	0x02	/* Active Namespace ID list */
#define NVMe_ID_NS	0x00	/* Identify Namespace data structure for the specified NSID */

uint64_t *pcie_ecam = NULL;
int16_t detected_bus_num = -1;
int16_t detected_device_num = -1;
int16_t detected_function_num = -1;
volatile uint64_t *nvme_base = NULL;
uint8_t nvme_mjr_num = 0;
uint8_t nvme_mnr_num = 0, nvme_ter_num = 0, nvme_irq = 0;
uint32_t SystemVariables;	// = 0x0000000000110000; // 0x110000 -> System Variables
uint64_t nvme_acqb; 	// = 0x0000000000171000; // 0x171000 -> 0x171FFF	4K admin completion queue base address
uint64_t nvme_ans; 	// 0x0000000000175000; // 0x175000 -> 0x175FFF	4K Namespace Data
uint64_t nvme_nsid;  	// 0x0000000000176000; // 0x176000 -> 0x176FFF	4K Namespace Identify Data

unsigned char check_xsdt_checksum(uint64_t *xsdt, uint32_t xsdt_length);
uint32_t check_mcfg_checksum(uint64_t *mcfg);
void check_all_buses(uint16_t start, uint16_t end);
int find_nvme_controller(uint16_t bus, uint8_t device, uint8_t function);
volatile uint64_t *get_nvme_base(uint16_t bus, uint8_t device, uint8_t function);
int check_nvme_vs(void);
uint8_t get_device_irq_num(uint16_t bus, uint8_t device, uint8_t function);
void *get_base_phy_addr(uint16_t bus, uint8_t device, uint8_t function);
void enable_pci_bus_mastering(void);
void disable_controller_interrupts(void);

int nvme_init(void *xsdp, uint8_t *sys_var_ptr)
{
	SystemVariables = (uint32_t) sys_var_ptr;
	nvme_acqb = SystemVariables + 0x61000;
	nvme_ans = SystemVariables + 0x65000;
	nvme_nsid = SystemVariables + 0x66000;
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

	/* Mark controller memory as un-cacheable */
	uncacheable_memory();

	/* check for a valid version number (bits 31:16 should be greater than 0) */
	if(check_nvme_vs() == 1) {
		return 1;
	}

	printk("@nvme version={d}.{d}.{d}\n", nvme_mjr_num, nvme_mnr_num, nvme_ter_num);

	/* Grab the IRQ of the device */
	nvme_irq = get_device_irq_num(detected_bus_num, detected_device_num, detected_function_num);

	printk("@nvme_irq={d}\n", nvme_irq);

	/* enable pcie bus mastering */
	enable_pci_bus_mastering();

	/* disable the controller if it's enabled */
	disable_nvme_controller();	


	/* configure AQA, ASQ, and ACQ */
	config_admin_queues();


	/* disable controller interrupts */
	disable_controller_interrupts();	


	// enable the controller
	enable_nvme_controller();	


	if(nvme_init_enable_wait() == 1) {
		printk("nvme: fatal error! CSTS.CFS (1) is not 0!\n");
		return 1;
	}


	/* create IO queues */
	create_io_queues();


	// Save the Identify Controller structure
	save_controller_struct();


	/* Save the Active Namespace ID list */
	save_active_nsid_list();


	/* Save the Identify Namespace data */
	save_namespace_data();


	
	// int32_t my_val = 5;

	// /* continue initialization in assembly code */
	// __asm__("mov rsi, %0\n\t"
	// 	"call nvme_asm_init\n\t"
	// 	"mov %1, ebx"
	// 	::"m" (nvme_base),
	// 	"m" (my_val):"rsi");

	// printk("nvme: controller is initialized! returned {d}\n", my_val);
	//
	// char data[512];
	// /* test: read a text file and it should print- Hello World! */
	// __asm__("mov rax, 421914624\n\t"
	// 	"mov rbx, 2\n\t"
	// 	"mov rcx, 1\n\t"
	// 	"mov rdx, 0\n\t"
	// 	"mov rdi, %0\n\t"
	// 	"call nvme_io"
	// 	::"m" (data):"rax", "rbx", "rcx", "rdx", "rdi");

	// printk("nvme: done reading data! Output=\n");

	// for (i = 0; i < 13; i++)
	// 	printk("{c}", data[i]);

	// printk("\n");
	
	printk("done!\n");

	return 0;
}

void uncacheable_memory(void)
{
	__asm__("mov rax, %0\n\t"
		"shr rax, 18\n\t"
		"and al, 0xF8\n\t"		// Clear the last 3 bits
		"mov rdi, 0x10000\n\t"		// Base of low PDE
		"add rdi, rax\n\t"
		"mov rax, [rdi]\n\t"
		"btc rax, 3\n\t"			// Clear PWT to disable caching
		"bts rax, 4\n\t"			// Set PCD to disable caching
		"mov [rdi], rax"
		::"m" (nvme_base):"rax","rdi");

}

void nvme_admin_wait(uint64_t acqb_copy)
{
	// for(int i = 0; i < 200; i++)
	// 	printk("{d}", *((char *) nvme_acqb + i));

	// printk("\n");

	uint64_t val;

	do{
		val = *(volatile uint64_t *) acqb_copy;
		// printk("@val={lld}\n", val);
	}while(val == 0);

	*(uint64_t *) acqb_copy = 0; // Overwrite the old entry
}	

void nvme_admin_savetail(uint8_t val, uint64_t * nvme_atail, uint32_t old_tail_val)
{
	uint32_t val_new = val;
	uint64_t acqb_copy = nvme_acqb;

	printk("@old_tail_val={d}\n", old_tail_val);


	*((volatile uint8_t *) nvme_atail) = val;	// Save the tail for the next command

	printk("@val={d}\n", val);



	*((uint32_t *) ((char *) nvme_base + 0x1000)) = val_new; // Write the new tail value

	// Check completion queue
	old_tail_val = (old_tail_val << 4);	// Each entry is 16 bytes
	old_tail_val = (uint8_t) old_tail_val + 8;	// Add 8 for DW3

	// printk("@old_tail_val={d}\n", old_tail_val);
	printk("@acqb_copy={llu}\n", acqb_copy);

	acqb_copy += old_tail_val;

	printk("@acqb_copy={llu}\n", acqb_copy);
	
	nvme_admin_wait(acqb_copy);
}

/*
 * nvme_admin -- Perform an Admin operation on a nvme controller
 */
void nvme_admin(uint32_t cdw0, uint32_t cdw1, uint32_t cdw10, uint32_t cdw11, uint64_t cdw6_7)
{
	uint64_t *nvme_asqb = SystemVariables + 0x60000;	// 0x0000000000170000;	// 0x170000 -> 0x170FFF	4K admin submission queue base address
	void *nvme_atail = SystemVariables + 0x0311;
	uint32_t tmp, a_tail_val;
	int64_t val2;
	uint8_t a_tail_val_8;

	// Build the command at the expected location in the Submission ring
	a_tail_val = *((volatile uint8_t*) nvme_atail); // Get the current Admin tail value
							       //
	printk("@a_tail_val={d}\n", a_tail_val);
	
	// printk("@admin tail value={d}\n", a_tail_val);

	a_tail_val = (a_tail_val << 6);			// Quick multiply by 64
							//
	a_tail_val_8 = a_tail_val;

	nvme_asqb = (uint64_t *) ((unsigned char *) nvme_asqb + a_tail_val_8);

	// printk("@nvme_asqb={p}\n", (void *) nvme_asqb);

	// Build the structure
	*(uint32_t *) nvme_asqb = cdw0;	// CDW0
	*((uint32_t *) ((char *) nvme_asqb + 4)) = cdw1;	// CDW1
	*((uint32_t *) ((char *) nvme_asqb + 8)) = 0;	// CDW2
	*((uint32_t *) ((char *) nvme_asqb + 12)) = 0;	// CDW3
	*((uint64_t *) ((char *) nvme_asqb + 16)) = 0;	// CDW4-5
	*((uint64_t *) ((char *) nvme_asqb + 24)) = cdw6_7;	// CDW6-7
	*((uint64_t *) ((char *) nvme_asqb + 32)) = 0;	// CDW8-9
	*((uint32_t *) ((char *) nvme_asqb + 40)) = cdw10;	// CDW10
	*((uint32_t *) ((char *) nvme_asqb + 44)) = cdw11;	// CDW11
	*((uint32_t *) ((char *) nvme_asqb + 48)) = 0;	// CDW12
	*((uint32_t *) ((char *) nvme_asqb + 52)) = 0;	// CDW13
	*((uint32_t *) ((char *) nvme_asqb + 56)) = 0;	// CDW14
	*((uint32_t *) ((char *) nvme_asqb + 60)) = 0;	// CDW15

	for(int i = 0; i < 64; i++)
		printk("{d}", *((volatile unsigned char *) nvme_asqb + i));

	printk("\n");


	// Start the Admin command by updating the tail doorbell
	a_tail_val = 0;
	a_tail_val = *((volatile uint8_t *) nvme_atail); // Get the current Admin tail value
	tmp = a_tail_val;	// Save the old Admin tail value for reading from the completion ring
	a_tail_val++;			// Add 1 to it
	
	// printk("@a_tail_val={d}\n", a_tail_val);

	if(a_tail_val>= 64)
		a_tail_val = 0;

	nvme_admin_savetail(a_tail_val, nvme_atail, tmp);
}

void save_namespace_data()
{
	uint32_t cdw0 = 0x00000006;	// CDW0 CID 0, PRP used (15:14 clear), FUSE normal (bits 9:8 clear), command Identify (0x06)
	uint32_t cdw1 = 1;	// CDW1 NSID
	uint32_t cdw11 = 0;	// CDW11 Ignored

	nvme_admin(cdw0, cdw1, NVMe_ID_NS, cdw11, nvme_nsid); 	// CDW10 CNS. CDW6-7 DPTR
}

void save_active_nsid_list(void)
{
	uint32_t cdw0 = 0x00000006;	// CDW0 CID 0, PRP used (15:14 clear), FUSE normal (bits 9:8 clear), command Identify (0x06)
	uint32_t cdw1 = 0;	// CDW1 Ignored
	uint32_t cdw11 = 0;	// CDW11 Ignored

	nvme_admin(cdw0, cdw1, NVMe_ANS, cdw11, nvme_ans); 	// CDW10 CNS. CDW6-7 DPTR
}

void save_controller_struct(void)
{
	uint32_t cdw0 = 0x00000006;	// CDW0 CID 0, PRP used (15:14 clear), FUSE normal (bits 9:8 clear), command Identify (0x06)
	uint32_t cdw1 = 0;	// CDW1 Ignored
	uint32_t nvme_ID_CTRL = 0x01;		// CDW10 CNS. Identify Controller data structure for the controller
	uint32_t cdw11 = 0;	// CDW11 Ignored
	uint64_t nvme_CTRLID = SystemVariables + 0x64000;	// 0x0000000000174000; // CDW6-7 DPTR. 0x174000 -> 0x174FFF	4K Controller Identify Data

	nvme_admin(cdw0, cdw1, nvme_ID_CTRL, cdw11, nvme_CTRLID);
}

void create_io_queues(void)
{	
	uint32_t val1 = 0x00010005;	// CDW0 CID (31:16), PRP used (15:14 clear), FUSE normal (bits 9:8 clear), command Create I/O Completion Queue (0x05)
	uint32_t val2 = 0; 	// CDW1 Ignored
	uint32_t val3 = 0x003F0001;		// CDW10 QSIZE 64 entries (31:16), QID 1 (15:0)
	uint32_t val4= 0x00000001;		// CDW11 PC Enabled (0)
	uint64_t nvme_iocqb = SystemVariables + 0x63000; 	// 0x0000000000173000;			// CDW6-7 DPTR. 0x173000 -> 0x173FFF	4K I/O Completion Queue Base Address

	// Create I/O Completion Queue
	nvme_admin(val1, val2, val3, val4, nvme_iocqb);

	// Create I/O Submission Queue
	val1 = 0x00010001;	// CDW0 CID (31:16), PRP used (15:14 clear), FUSE normal (bits 9:8 clear), command Create I/O Submission Queue (0x01)
	val3 = 0x003F0001;		// CDW10 QSIZE 64 entries (31:16), QID 1 (15:0)
	val4 = 0x00010001;		// CDW11 CQID 1 (31:16), PC Enabled (0)
	uint64_t nvme_iosqb = SystemVariables + 0x62000; 	// 0x0000000000172000;	// CDW6-7 DPTR. 0x172000 -> 0x172FFF	4K I/O Submission Queue Base Address

	nvme_admin(val1, val2, val3, val4, nvme_iosqb);
}

int nvme_init_enable_wait(void)
{
	char nvme_csts =  0x1C; // 4-byte controller status property
	void* addr = (void *) ((char *) nvme_base + nvme_csts);
	uint32_t val;

	do{
		val = *((volatile uint32_t *) addr);

		if((val & 0x2)!= 0x0)	// CSTS.CFS (1) should be 0. If not the controller has had a fatal error
		{
			return 1;
		}
	}while((val & 0x1) != 0x1);	// Wait for CSTS.RDY (0) to become '1'

	return 0;
}

void enable_nvme_controller(void)
{
	uint32_t val = 0x00460001;		// set iocqes (23:20), iosqes (19:16), and en (0)
	char nvme_cc = 0x14;	// 4-byte controller configuration property
	void* addr = (void *) ((char *) nvme_base + nvme_cc);

	*((uint32_t *) addr) = val;	// write the new cc value and enable controller
	
	printk("@cc={d}\n", *((uint32_t *) addr));
}

void disable_controller_interrupts(void)
{
	uint32_t val = 0xffffffff;		// mask all interrupts
	char nvme_intms = 0x0C; // 4-byte interrupt mask set
	void* addr_intms = (void *) ((char *) nvme_base + nvme_intms);

	*((uint32_t *) addr_intms) = val;
}

/* 
 * configure AQA, ASQ, and ACQ
 */
void config_admin_queues(void)
{
	char nvme_aqa = 0x24;		// 4-byte Admin Queue Attributes
	void* addr_aqa = (void *) ((char *) nvme_base + nvme_aqa);
	uint32_t value = 0x003f003f;		// 64 commands each for ACQS (27:16) and ASQS (11:00)

	char nvme_asq = 0x28;	// 8-byte admin submission queue base address
	uint64_t nvme_asqb = SystemVariables + 0x60000; 	// 0x0000000000170000; // 0x170000 -> 0x170FFF	4K admin submission queue base address
	void* addr_asq = (void *) ((char *) nvme_base + nvme_asq);

	char nvme_acq = 0x30; // 8-byte admin completion queue base address
	void* addr_acq = (void *) ((char *) nvme_base + nvme_acq);

	 // memset(nvme_acqb, 0, 4096);

	// printk("@value={d}\n", value);

	*((uint32_t *) addr_aqa) = value;

	// printk("@value={d}\n", *((uint32_t *) addr_aqa));


	*((uint64_t *) addr_asq) = nvme_asqb;	// ASQB 4K aligned (63:12)

	// printk("@ASQ={llu}\n", *((uint64_t *) addr_asq));
	

	*((uint64_t *) addr_acq) = nvme_acqb;	// ACQB 4K aligned (63:12)
	
	// printk("@ACQ={llu}\n", *((uint64_t *) addr_acq));
}

void disable_nvme_controller(void)
{
	char nvme_cc = 0x14;	// 4-byte controller configuration property
	void* addr = (void *) ((char *) nvme_base + nvme_cc);
	uint32_t value;

	value = *((volatile uint32_t *) addr);

	// printk("@value={d}\n", value);

	if((value & 0x1) != 0x0) {		// clear CC.EN bit 0 to '0'
		value = (value & 0xfffffffe);
		*((uint32_t *) addr) = value;
	}


	// printk("@value={d}\n", *((uint32_t *) addr));
}

void enable_pci_bus_mastering(void)
{
	void *phy_addr;
	int32_t value;

	phy_addr = get_base_phy_addr(detected_bus_num, detected_device_num, detected_function_num);

	phy_addr = (void *) ((char *) phy_addr + (1 * 4));	/* get status/command from pcie device's register #1 */

	value = *((volatile int32_t *) phy_addr);

	__asm__("mov eax, %0\n\t"
		"bts eax, 2\n\t"
		"mov %0, eax"
		::"m" (value):"eax");
	
	*((int32_t *) phy_addr) = value;	/* write value to pcie device's register #1 */

}


void *get_base_phy_addr(uint16_t bus, uint8_t device, uint8_t function)
{
	void *phy_addr;

	phy_addr = (void *) ((uint64_t) pcie_ecam + (((uint32_t) bus) << 20 | ((uint32_t) device) << 15 | ((uint32_t) function) << 12));

	return phy_addr;
}


uint8_t get_device_irq_num(uint16_t bus, uint8_t device, uint8_t function)
{
	void *phy_addr;
	unsigned char value;

	phy_addr = get_base_phy_addr(bus, device, function);

	phy_addr = (void *) ((char *) phy_addr + (15 * 4));	/* device's IRQ number from PCIe Register 15 (IRQ is bits 7-0). Multiplying by 4 because each register consists of 4 bytes */

	value = *(volatile uint8_t *) phy_addr;

	return (uint8_t) value;
}

/* returns 0 on success else 1 on failure */
int check_nvme_vs(void)
{
	void *phy_addr;
	uint32_t value, tmp;
	uint8_t mjr_num = 0, mnr_num, ter_num;

	phy_addr = (void *) ((char *) nvme_base + NVMe_VS);

	value = *((uint32_t *) phy_addr);

	__asm__("mov eax, %0\n\t"
		"ror eax, 16\n\t" /* rotate eax so MJR is bits 15:00 */
		"mov %1, al\n\t"
		"mov %2, eax"	  /* copy eax to tmp C variable */
		::"m" (value),
		"m" (mjr_num),
		"m" (tmp):"eax");

	if (mjr_num < 1) {
		printk("error: nvme: invalid major version number!\n");
		return 1;
	}

	nvme_mjr_num = mjr_num;	/* store major version num */

	__asm__("mov eax, %0\n\t" /* copy tmp c variable value to eax */
		"rol eax, 8\n\t" /* rotate eax so MNR is bits 07:00 */
		"mov %1, al\n\t"
		"rol eax, 8\n\t" /* rotate eax so TER is bits 07:00 */
		"mov %2, al"
		::"m" (tmp),
		"m" (mnr_num),
		"m" (ter_num):"eax");

	nvme_mnr_num = mnr_num;	/* store minor and tertiary ver num */
	nvme_ter_num = ter_num;

	return 0;
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
