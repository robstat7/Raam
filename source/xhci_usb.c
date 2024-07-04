#include "string.h"
#include <string.h>
#include "printk.h"
#include <stdint.h>

struct enable_slot_command_trb {
	unsigned long long rsvdz_1 :64;
	unsigned long long rsvdz_2 :32;
	unsigned long long c: 1;
	unsigned long long rsvdz_3 :9;
	unsigned long long trb_type :5;
	unsigned long long slot_type :5;
	unsigned long long rsvdz_4 :11;
} enable_slot_command_trb_fields;

struct event_ring_segment_table {

	unsigned long long ring_seg_base_addr :64;
	unsigned long long ring_seg_size :16;
	unsigned long long rsvdz :48;
} event_ring_segment_table_t;


volatile uint64_t *pcie_ecam = NULL;
volatile uint64_t *xhci_base = NULL;
int16_t detected_bus_num = -1;
int16_t detected_device_num = -1;
int16_t detected_function_num = -1;
uint8_t capability_register_length = 0;
/* command ring dequeue pointer */
volatile uint64_t *command_ring_base = NULL;	// 4k bytes command ring
volatile uint64_t *command_ring_dequeue_pointer = NULL;
volatile char *operational_registers_base = NULL;
volatile uint64_t *device_context_base_address_array = NULL;
int device_context_base_address_array_size = 24;	// in bytes
volatile uint64_t *event_ring_segment_0 = NULL;		/* 4k bytes event ring segment #0 */
int event_ring_segment_0_size = 256;			/* 256 TRBs */

volatile uint64_t *get_base_phy_addr(void);
volatile uint32_t *get_doorbell_reg_0_base(void);
unsigned char check_xsdt_checksum(uint64_t *xsdt, uint32_t xsdt_length);
volatile uint64_t *get_xhci_base(void);
uint32_t check_mcfg_checksum(uint64_t *mcfg);
void check_all_buses(uint16_t start, uint16_t end);
int find_xhci_controller(uint16_t bus, uint8_t device, uint8_t function);
uint8_t get_cap_reg_len(void);
volatile uint64_t *find_first_64_byte_aligned_address(char *sys_var_ptr);

int xhci_init(void *xsdp, uint8_t *sys_var_ptr)
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

	/* enumerate pcie buses to find the xhci controller */
	check_all_buses(start_bus_num, end_bus_num);

	if(detected_bus_num != -1 && detected_device_num != -1 && detected_function_num != -1) {
		printk("found xhci controller! bus num={d}, device num={d}, function num={d}\n\n", detected_bus_num, detected_device_num, detected_function_num);
	} else {
		printk("couldn't found the xhci controller!\n");
		return 1;
	}

	xhci_base = get_xhci_base();

	printk("@xhci_base = {p}\n", (void *) xhci_base);

	// enable bus mastering
	enable_pci_bus_mastering();

	/* get the Capability Register Length */
	capability_register_length = get_cap_reg_len();

	printk("@capability_register_length = {d}\n", capability_register_length);

	/* find the operational registers base */
	operational_registers_base = (char *) xhci_base + capability_register_length;

	printk("@operational_registers_base = {p}\n", (void *) operational_registers_base);

	/* reset the host controller */
	reset_controller();

	/* set the maximum number of enabled device slots */
	set_max_slots_en();

	/* allocate 24 bytes for device context base address array consisting of 3 entries of 64-bit each. Entry 0 is reserved */
	device_context_base_address_array = find_first_64_byte_aligned_address(sys_var_ptr);

	printk("@device_context_base_address_array = {p}\n", device_context_base_address_array);

	/* set Device Context Base Address Array Pointer Register (DCBAAP) */
	set_dcbaap_reg();

	/* set command ring control register (CRCR) */
	set_cmd_ring_cntrl_reg();

	/* set the event ring segment 0 base address */
	event_ring_segment_0 = (uint64_t *) (char *) command_ring_base + 4096;


	/* initialize the only event ring segment table entry */
	event_ring_segment_table_t.ring_seg_base_addr = event_ring_segment_0; 
	event_ring_segment_table_t.ring_seg_size = event_ring_segment_0_size;

	/* find the runtime base */
	volatile uint32_t *rtsoff = (uint32_t *) ((char *) xhci_base + 0x18);	/* RTSOFF register address */
	uint32_t value = *rtsoff;
	printk("@rtsoff reg = {p}\n", (void *) value);

	volatile uint32_t *runtime_base = (uint32_t *) ((char *) xhci_base + value);

	printk("@runtime_base = {p}\n", (void *) runtime_base);

	/* find the Event Ring Segment Table Size Register (ERSTSZ) address */
	volatile uint32_t *erstsz = (uint32_t *) ((char *) runtime_base + 0x028 + (32 * 0));
	*erstsz = event_ring_segment_0_size;

	printk("@erstsz = {d}\n", *erstsz);

	/* set Event Ring Dequeue Pointer Register (ERDP) to event_ring_segment_0 */
	volatile uint64_t *erdp = (uint64_t *) ((char *) runtime_base + 0x038 + (32 * 0));
	*erdp = event_ring_segment_0;

	printk("@erdp reg = {p}\n", (void *) *erdp);



		/* find the event ring segment table base address register (ERSTBA) address */
		volatile uint64_t *erstba = (uint64_t *) ((char *) runtime_base + 0x030 + (32 * 0));

		*erstba = event_ring_segment_0;

		printk("@&event_ring_segment_table_t = {p}\n", (void *) &event_ring_segment_table_t);
		printk("@erstba reg = {p}\n", (void *) *erstba);



	run_the_controller();


	check_device_connected_to_port_2();

	reset_port_2();

	enable_slot_command_trb_fields.trb_type = 0x9;	// enable slot command
	enable_slot_command_trb_fields.c = 1;	// cycle bit

	printk("@enable_slot_command_trb_fields.slot_type = {d}\n", enable_slot_command_trb_fields.slot_type);


		memcpy((void *) command_ring_base, (void *) &enable_slot_command_trb_fields, sizeof(enable_slot_command_trb_fields));
	struct enable_slot_command_trb *trb1 = (struct enable_slot_command_trb *) command_ring_base;


printk("@trb1->trb_type = {d}\n", trb1->trb_type);

	volatile uint32_t *doorbell_reg_0 = get_doorbell_reg_0_base();



	*doorbell_reg_0 = 0;	// ring the HC doorbell. Here, DB Target is HC Command ('0')

	printk("@rang the HC doorbell!\n");

	while(1)
	{
		if(*event_ring_segment_0 != 0)
			printk("@FOUND EVENT TRB!!! 64bits value = {llu}\n", *event_ring_segment_0);

	}

	return 0;
}

void enable_pci_bus_mastering(void)
{
	volatile void *phy_addr;
	int32_t value;

	phy_addr = (void *) get_base_phy_addr();

	phy_addr = (volatile void *) ((char *) phy_addr + (1 * 4));	/* get status/command from pcie device's register #1 */

	value = *((volatile int32_t *) phy_addr);

	__asm__("mov eax, %0\n\t"
		"bts eax, 2\n\t"
		"mov %0, eax"
		::"m" (value):"eax");
	
	*((volatile int32_t *) phy_addr) = value;	/* write value to pcie device's register #1 */

	printk("@@@bus_mastering_enable func: value read = {d}\n", *((volatile uint32_t *) phy_addr));

}

void set_cmd_ring_cntrl_reg(void)
{
	volatile uint64_t *crcr = (uint64_t *) ( (char *) operational_registers_base + 0x18);
	printk("@crcr = {p}\n", (void *) crcr);

	uint64_t addr = (uint64_t) ((char *) device_context_base_address_array + 64);	// next 64-byte aligned address in the system variables memory region

	printk("@set_cmd_ring_cntrl_reg: addr = {p}\n", (void *) addr);

	// printk("@crcr reg before writing:{p}\n", (void *) *crcr);

	*crcr = addr;

	// // Explicit memory barrier for use with GCC.
	// asm volatile ("": : :"memory");

	// /* force serialization */
	// __asm__("mov rax, cr0\n\t"
	// 	"mov cr0, rax"
	// 	:::"rax");

	// /* issue a serializing instruction to force a memory barrier operation in hardware */
	// __asm__("cpuid"
 	// 	:::);


	// __asm__("mov rax, %1\n\t"
	// 	"mov rbx, %0\n\t"
	// 	"mov [rbx], rax\n\t"
	// 	"cpuid"
	// 	::"m" (crcr),
	// 	  "m" (addr):"rax","rbx");

	// Explicit memory barrier for use with GCC.
	// asm volatile ("": : :"memory");



	// printk("@crcr reg new value = {p}\n", (void *) *crcr);

		command_ring_base = (uint64_t *) addr;
	command_ring_dequeue_pointer = (uint64_t *) addr;
}

void set_dcbaap_reg(void)
{
	volatile uint64_t *dcbaap = (uint64_t *) (operational_registers_base + 0x30);	// 64-bit Device Context Base Address Array Pointer Register (DCBAAP)

	*dcbaap = (uint64_t) device_context_base_address_array;

	printk("@DCBAAP reg new value = {p}\n", (void *) *dcbaap);
}


volatile uint64_t *find_first_64_byte_aligned_address(char *sys_var_ptr)
{
	volatile char *addr = sys_var_ptr - 1;

	while((uint64_t) addr % 64 != 0) {
		addr++;
	}

	return (volatile uint64_t *) addr;
}

void reset_controller(void)
{
	uint32_t value;

	value = *(volatile uint32_t *) operational_registers_base;	// 32-bit USBCMD register (Operational Base+ (00h))

	value |= 0x2;	// set Host Controller Reset (HCRST) bit

	volatile uint32_t *usbsts = (uint32_t *) (operational_registers_base + 0x04); // USB Status Register (USBSTS)

	/* wait until the Controller Not Ready (CNR) flag in the USBSTS is ‘0’ */
	do {
		value = *usbsts;

		value >>= 11;

	} while(value % 2 == 1); 

	printk("@@@controller reset completed!\n");
}

void set_max_slots_en(void)
{
	volatile uint32_t *config_reg = (uint32_t *) (operational_registers_base + 0x38);
	uint32_t value;

	value = *config_reg;

	// printk("@config_reg initial value = {d}\n", value);

	value |= 0x02;	// CONFIG(7:0) is 0x2 for maximum no. of 2 device slots
	
	*config_reg = value;

	printk("@xhci_usb: set_max_slots_en: config_reg = {d}\n", *config_reg);
}


int find_xhci_controller(uint16_t bus, uint8_t device, uint8_t function) {	
	volatile uint64_t *phy_addr;
	uint32_t value;

	phy_addr = (uint64_t *) ((uint64_t) pcie_ecam + (((uint32_t) bus) << 20 | ((uint32_t) device) << 15 | ((uint32_t) function) << 12));

	phy_addr = (uint64_t *) ((char *) phy_addr + 8);

	value = *(volatile uint32_t *) phy_addr;
	
	value = value >> 8;

	if(value == 0xC0330) /* class code = 0x0C, subclass code = 0x03, prog if = 0x30 */
		return 0;
	else
		return 1;
}

uint16_t get_vendor_id(uint16_t bus, uint8_t device, uint8_t function)
{
	volatile uint64_t *phy_addr;
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

	res = find_xhci_controller(bus, device, function);	


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
			if(device == 13)	/* skip the thuderbolt 4 usb controller */
			{
				continue;
			}

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

	length = *((volatile uint32_t *) ((unsigned char *) mcfg + 4));

	for(i = 0; i < length; i++)
		sum += ((volatile unsigned char *) mcfg)[i];

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
	sum += ((volatile char *) xsdt)[i];
    }

    return sum;
}

volatile uint64_t *get_xhci_base(void)
{
	volatile char *phy_addr;
	volatile uint32_t *bar0_value, bar1_value;
	volatile uint64_t *base_addr;

	phy_addr = (volatile char *) ((uint64_t) pcie_ecam + (((uint32_t) detected_bus_num) << 20 | ((uint32_t) detected_device_num) << 15 | ((uint32_t) detected_function_num) << 12));

	phy_addr = phy_addr + (4 * 4);	/* Multiplying by 4 because each register consists of 4 bytes */

	/* read register 4 for BAR0 */
	bar0_value = *(volatile uint32_t *) phy_addr;

	/* read regiser #5 for BAR1 */
	phy_addr += 4;
	bar1_value = *(volatile uint32_t *) phy_addr;

	base_addr = bar1_value;
	base_addr = (uint64_t) base_addr << 32;		/* left shift 32 bits */
	base_addr = (uint64_t) base_addr | (uint64_t) bar0_value;

	base_addr = (uint64_t) base_addr & 0xfffffffffffffff0;		/* clear the lowest 4 bits */

	return base_addr;
}

volatile uint64_t *get_base_phy_addr(void)
{
	volatile uint64_t *phy_addr;

	phy_addr = (uint64_t *) ((uint64_t) pcie_ecam + (((uint32_t) detected_bus_num) << 20 | ((uint32_t) detected_device_num) << 15 | ((uint32_t) detected_function_num) << 12));

	return phy_addr;
}
uint8_t get_cap_reg_len(void)
{

	volatile uint8_t *addr = (char *) xhci_base;
	uint8_t value = *addr;

	return value;
}

void run_the_controller(void)
{
	uint32_t value = *(volatile uint32_t *) operational_registers_base;

	value |= 0x1;

	*(volatile uint32_t *) operational_registers_base = value;

	printk("@usb_cmd reg = {p}\n", (void *) *(volatile uint32_t *) operational_registers_base);
}

void check_device_connected_to_port_2(void)
{
	volatile uint32_t *addr = (uint32_t *) ((char *) operational_registers_base + (0x400 + (0x10 * (2-1))));

	while(1) {
		uint32_t value = *addr;

		if((value >> 17) % 2 == 1)	/* bit #17 (starting from #0) is 1. CSC = 1 implies change in CCS. */
		{
			printk("@xhci_usb: change in CSC detected!\n");
			if(value % 2 == 1)	/* bit #0 (CCS) is 1. 1 implies "a device is connected". */
			{
				printk("@xhci_usb: usb device detected!\n");

				break;
			}
		}
	}

}

void reset_port_2(void)
{
	// PORTSC register
	volatile uint32_t *addr = (uint32_t *) ((char *) operational_registers_base + (0x400 + (0x10 * (2-1))));

	uint32_t value = *addr;

	value |= 0x10;

	*addr = value;



	while(1) {

	value = *addr;

	if(((value >> 21) % 2 == 1) && ((value >> 1) %2 == 1) && ((value >> 4) % 2 == 0)) {
		if((value >> 5) % 8 == 0) {
			printk("@port reset completed!\n");
			break;
		}
	}
	}
}

volatile uint32_t *get_doorbell_reg_0_base(void)
{
	volatile uint32_t *addr = (uint32_t *) ((char *) xhci_base + 0x14);	// DBOFF register
	uint32_t address_val = *addr;

	return (uint32_t *) address_val;
}
