#include <raam/pic.h>
#include <raam/printk.h>
#include <raam/port_io.h>

void pic_init(void)
{
	/* We have to remap the IRQs to be able to process
	 * hardware-related interrupt requests and to service
	 * exceptions as well.
	 */
	

	printk("@pic:c: beginning initialization  ");
	/* set up cascading mode */
	outportb(PIC_MASTER_CMD, 0x10 + 0x01);
	outportb(PIC_SLAVE_CMD,  0x10 + 0x01);
	/* Setup master's vector offset */
	outportb(PIC_MASTER_DATA, 0x20);
	/* Tell the slave its vector offset */
	outportb(PIC_SLAVE_DATA, 0x28);
	/* Tell the master that he has a slave */
	outportb(PIC_MASTER_DATA, 4);
	outportb(PIC_SLAVE_DATA, 2);
	/* Enabled 8086 mode */
	outportb(PIC_MASTER_DATA, 0x01);
	outportb(PIC_SLAVE_DATA, 0x01);

	printk("@pic.c: resetting masks  ");
	outportb(PIC_MASTER_DATA, 0);
	outportb(PIC_SLAVE_DATA, 0);
	printk("@pic.c: init done.  ");
}

void pic_send_eoi(uint8_t irq)
{
	if(irq >= 8)
		outportb(PIC_SLAVE_CMD, PIC_CMD_EOI);
	outportb(PIC_MASTER_CMD, PIC_CMD_EOI);
}
