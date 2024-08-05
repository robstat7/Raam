/*
 * PS/2 controller driver.
 */
#include "ps2.h"
#include <stdint.h>

uint64_t timer_count = 50000;		/* timer count */

uint32_t read_ioapic_register(const uint32_t *apic_base, const uint8_t offset);
void write_ioapic_register(const uint32_t *apic_base, const uint8_t offset, const uint32_t val);


/* PS/2 controller initialization */
int init_ps2_controller(uint64_t *xsdp)
{
	setup_timer();

	uint64_t *madt = get_madt_pointer(xsdp);

	if(madt == NULL)
		return 1;


	uint32_t value = *(volatile uint32_t *) ((char *) madt + 40);

	// printk("@madt flags = {d}\n", value);




	// I/O APIC address is MADT base address + 44 + (32 * 8) i.e. madt + 300

	uint32_t ioapic_addr = *(volatile uint32_t *) ((char *) madt + 304);


	// printk("@ioapic addr = {p}\n", (void *) ioapic_addr);
	

	uint32_t ioapicid = read_ioapic_register(ioapic_addr, 0);
	
	// printk("@IOAPICID= {p}\n", (void *) ioapicid);


	uint32_t ioredtbl0 = read_ioapic_register(ioapic_addr, 0x10);
	uint32_t ioredtbl1 = read_ioapic_register(ioapic_addr, 0x12);
	uint32_t ioredtbl2 = read_ioapic_register(ioapic_addr, 0x14);

	// printk("@ioredtbl0 = {p}\n", (void *) ioredtbl0);
	// printk("@ioredtbl1 = {p}\n", (void *) ioredtbl1);
	// printk("@ioredtbl2 = {p}\n", (void *) ioredtbl2);

	// write_ioapic_register(ioapic_addr, 0x12, 0x21); //IOREDTBL1 (bits 31:0) = vector nr. (bits 0-7) = 0x21, other bits = 0
	write_ioapic_register(ioapic_addr, 0x12, 0x10021); //IOREDTBL1 (bits 31:0) = vector nr. (bits 0-7) = 0x21, bit 16 = 1, other bits = 0
	write_ioapic_register(ioapic_addr, 0x13, 0xf000000); //IOREDTBL1 (bits 63:32) = logical destination addr (bits 59-56) = 0xf (APIC ID), other bits = 0

	/* TODO: fix IRQ1 is not firing. */


	/* disable devices */

	if(send_bytes_to_dev(0x64, 0xad) == 1)
		return 1;

	if(send_bytes_to_dev(0x64, 0xa7) == 1)
		return 1;

	inportb(0x60);	/* flush the output buffer */


	/* read and set the controller configuration byte */
	
	uint8_t cc_byte = read_controller_configuration_byte();

	// printk("@contr 1 cc_byte = {p}\n", (void *) cc_byte);

	int dual_channel_controller = is_dual_channel_controller(cc_byte);

	// printk("@contr 1 dual_channel_controller = {d}\n", dual_channel_controller);

	cc_byte &= 0xbc;	/* disable all IRQs and disable translation (clear bits 0, 1 and 6) */

	// printk("@contr 1 new cc_byte = {p}\n", (void *) cc_byte);

	if(send_bytes_to_dev(0x64, 0x60) == 1)
		return 1;

	if(send_bytes_to_dev(0x64, cc_byte) == 1)
		return 1;

	/* determine if there are 2 channels */
	if(dual_channel_controller == 1) {
		if(send_bytes_to_dev(0x64, 0xa8) == 1)	/* enable the second PS/2 port */
			return 1;

		// printk("@ps2: second PS/2 port is enabled!\n");

		uint8_t cc_byte = read_controller_configuration_byte();
		// printk("@contr 2 cc_byte = {p}\n", (void *) cc_byte);
		int dual_channel_controller = !is_dual_channel_controller(cc_byte);
		// printk("@contr 2 dual_channel_controller = {d}\n", dual_channel_controller);

		if(dual_channel_controller) {
			if(send_bytes_to_dev(0x64, 0xa7) == 1)	/* disable the second PS/2 port */
				return 1;

			// printk("@ps2: second PS/2 port disabled!\n");
			// printk("@ps2: only 1 channel is present!\n");
		} else {
			// printk("@ps2: 2 channels are present!\n");
		}
	}

	/* enable the first port */

	if(send_bytes_to_dev(0x64, 0xae) == 1)
			return 1;

	// printk("@ps2: first port enabled!\n");


	/* enable interrupts */

	if(dual_channel_controller)
		cc_byte |= 0x3;	/* set bit 0 and 1 */
	else
		cc_byte |= 0x1; /* set bit 0 */

	if(send_bytes_to_dev(0x64, 0x60) == 1)
			return 1;

	if(send_bytes_to_dev(0x64, cc_byte) == 1)
			return 1;

	// printk("@ps2: interrupts enabled!\n");


	/* reset devices */

	if(send_bytes_to_dev(0x60, 0xff) == 1)
		return 1;

	 while(inportb(0x60) != 0xfa);

	 if(dual_channel_controller) {
	 	 if(send_bytes_to_dev(0x64, 0xd4) == 1)
	 		 return 1;

	 	 if(send_bytes_to_dev(0x60, 0xff) == 1)
	 		return 1;
	 
	 	 while(inportb(0x60) != 0xfa);
	  }


	// printk("@ps2: device(s) reset completed!\n");

	return 0;
}

void setup_timer(void)
{
	uint32_t t_mode = 0x0;	/* one shot timer mode */

	/* set timer divide value */
	set_divide_value(0xb); /* divide by 1 (111) */

	set_mode(t_mode);	/* set timer mode */ 
}

int send_bytes_to_dev(uint16_t port_id, uint8_t value)
{
	int flag = 1;

	set_initial_count(timer_count);	/* set initial count of timer */

	while(read_current_count() > 0) {
		/* Poll bit 1 of the Status Register ("Input buffer empty/full") until it becomes clear */
		if((inportb(0x64) & 0x2) == 0) {
			flag = 0;
			break;
		}
	}

	if(flag == 1)
	{
		printk("@ps2: error: time-out expired while checking for clear status of the bit #1 of status register!\n");
	} else {
		outportb(port_id, value);
	}

	return flag;
}

/* read the configuration byte */
uint8_t read_controller_configuration_byte(void)
{
	if(send_bytes_to_dev(0x64, 0x20) == 1)
		return 1;

	return inportb(0x60);
}

/* 
 * check bit 5 of controller configuration byte to determine if it is a "dual channel" controller.
 * set = true
 * clear = false
 */
int is_dual_channel_controller(uint8_t cc_byte)
{
	int dual_channel_controller = 0;

	if((cc_byte & 0x20) == 0x20)
		dual_channel_controller = 1;

	return dual_channel_controller;
}

void write_ioapic_register(const uint32_t *apic_base, const uint8_t offset, const uint32_t val) 
{
    /* tell IOREGSEL where we want to write to */
    *(volatile uint32_t*)(apic_base) = offset;
    /* write the value to IOWIN */
    *(volatile uint32_t*)(apic_base + 0x10) = val; 
}
 
uint32_t read_ioapic_register(const uint32_t *apic_base, const uint8_t offset)
{
    /* tell IOREGSEL where we want to read from */
    *(volatile uint32_t*)(apic_base) = offset;
    /* return the data from IOWIN */
    return *(volatile uint32_t*)(apic_base + 0x10);
}
